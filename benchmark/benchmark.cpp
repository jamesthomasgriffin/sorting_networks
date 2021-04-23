#include <algorithm>       // For std::sort
#include <assert.h>        // For assert
#include <chrono>          // For std::chrono::high_resolution_clock and related classes
#include <cstdlib>         // For qsort
#include <fstream>         // For std::ofstream
#include <iostream>        // For std::cout
#include <random>          // For std::default_random_engine
#include <vector>          // For std::vector

#include "vectorclass.h"   // Agner Fog's library using x86 simd intrinsics

#include "static_sort.h"   // Vectorized/static-sort (available on github.com)

#include "network_sort.h"  // Our implementation (for simd should be included after vectorclass.h)

/*

This offers rudimentary benchmarking, reordering the methods could offer 10-20% change in performance.

* Make sure to enable AVX2

*/

// an example of an alternative method of swapping elements, used as a template parameter in one of our benchmarks
// there is no consistently observed performance improvement (on JTG's laptop at least)
class IntSwap {
public:
  inline IntSwap(int &x, int &y) {
    int dx = x, dy = y, tmp;
    tmp = x = dx < dy ? dx : dy;
    y ^= dx ^ tmp;
  }
};

template <class C> 
static inline void fill_with_random_bits(C &container) {
  std::default_random_engine eng{123};
  for (auto &v : container)
    v = eng();
}

template <class C>
static inline void check_sorted_sequences(C &container, int num_elements) {
  auto const total_elements = container.size();
  for (int i = 0; i < total_elements - num_elements; i += num_elements) {
    bool ordered = true;
    for (int j = 0; j < num_elements - 1; ++j)
      if (container[i + j] > container[i + j + 1])
        ordered = false;
    if (!ordered) {
      std::cout << "Failure at sequence " << i / num_elements << "\n";
      for (int j = 0; j < num_elements; ++j)
        std::cout << container[i + j] << ' ';
      std::cout << '\n';
      return;
    }
  }
}

template<class F> auto benchmark(F &&f, int n_tests, int n_elements) {

  std::vector<int> data(n_tests * n_elements);
  fill_with_random_bits(data);

  auto const start_time = std::chrono::high_resolution_clock::now();

  f(data, n_tests, n_elements);

  auto const duration = std::chrono::high_resolution_clock::now() - start_time;

  auto const duration_in_ns = duration.count();

  check_sorted_sequences(data, n_elements);

  return static_cast<double>(duration_in_ns);
}

int qsort_cmp(void const *a, void const *b) {
  return (*(int *)a > *(int *)b) - (*(int *)a < *(int *)b);
}

constexpr int n_benchmarks = 6;

template<int NumElements, typename OS>
void run_benchmarks(int num_tests, OS &ostr) {
  ostr << NumElements;

  std::cout << sorting_networks::SortingNetwork<NumElements>{} << '\n';

  double results[n_benchmarks]{};
  auto result_ix = 0;

  // Vanilla std::sort
  results[result_ix++] = benchmark(
      [](auto &data, auto n_tests, auto n_elements) {
        auto ptr = data.data();
        for (int i = 0; i < n_tests * n_elements; i += NumElements)
          std::sort(ptr + i, ptr + i + NumElements);
      },
      num_tests, NumElements);

  // qsort
  results[result_ix++] = benchmark(
      [](auto &data, auto n_tests, auto n_elements) {
        auto ptr = data.data();
        for (int i = 0; i < n_tests * n_elements; i += NumElements)
          qsort(ptr + i, NumElements, sizeof(int), qsort_cmp);
      },
      num_tests, NumElements);

  // sorting network from Vectorized/static-sort, uses similar methods to our
  // own
  results[result_ix++] = benchmark(
      [](auto &data, auto n_tests, auto n_elements) {
        StaticSort<NumElements> staticSort;
        auto ptr = data.data();
        for (int i = 0; i < n_tests * n_elements; i += NumElements)
          staticSort(ptr + i);
      },
      num_tests, NumElements);

  // our sorting network, uses Batcher's odd / even merge sort
  results[result_ix++] = benchmark(
      [](auto &data, auto n_tests, auto n_elements) {
        sorting_networks::SortingNetwork<NumElements> network;

        auto ptr = data.data();
        for (int i = 0; i < n_tests * n_elements; i += NumElements)
          network(ptr + i);
      },
      num_tests, NumElements);

  // the same network, however with a different method of swapping integers
  results[result_ix++] = benchmark(
      [](auto &data, auto n_tests, auto n_elements) {
        using network_type = typename sorting_networks::SortingNetwork<
            NumElements>::template Sort<int *, IntSwap, 0, 1, NumElements>;
        int *ptr = data.data();
        for (int i = 0; i < n_tests * n_elements; i += NumElements) {
          int *ptr2 = ptr + i;
          network_type network(ptr2);
        }
      },
      num_tests, NumElements);

  // the same network again, however this sorts 8 sequences at once using AVX2
  // instructions
  results[result_ix++] = benchmark(
      [](auto &data, auto n_tests, auto n_elements) {
        assert(n_tests % 8 == 0);

        typename sorting_networks::SortingNetwork<NumElements>::SIMD8 network{};

        auto ptr = data.data();
        for (int i = 0; i < n_tests * n_elements; i += 8 * NumElements)
          network(ptr + i);
      },
      num_tests, NumElements);

  assert(result_ix == n_benchmarks);

  // Output results
  for (int i = 0; i < n_benchmarks; ++i)
    ostr << ", " << results[i] / num_tests;
  ostr << "\n";
}

template <auto Start, auto End, auto Inc, class F>
constexpr void constexpr_for(F &&f) {
  if constexpr (Start < End) {
    f(std::integral_constant<decltype(Start), Start>());
    constexpr_for<Start + Inc, End, Inc>(f);
  }
}

int main(int argc, const char *argv[]) {
  std::ofstream ostr{"output.csv"};

  // Output column headers
  std::string names[n_benchmarks] = {"std::sort",        "qsort",
                                     "static_sort",      "jtg_sort",
                                     "jtg_sort_intswap", "jtg_sort_avx2"};
  ostr << "NumElements";
  for (auto const &name : names)
    ostr << ", " << name;
  ostr << '\n';

  constexpr int Start = 2, End = 17;
  constexpr_for<Start, End, 1>(
      [&ostr](auto N) { run_benchmarks<N>((1000000 / N) * 8, ostr); });

  return 0;
}
