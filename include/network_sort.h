#pragma once

#ifdef VECTORCLASS_H
#define NETWORK_SORT_SIMD_AVAILABLE

#include <array>  // std::array for reordering data ready for simd sorting

#endif

#include <utility>

namespace sorting_networks {

static constexpr int round_down_to_power_of_2(int n) {
  int result = 1;
  while (2 * result <= n)
    result *= 2;
  return result;
}


static constexpr int size_of_first_block(int n) {
  return round_down_to_power_of_2((2 * n) / 3);
}

namespace swappers {

template <class V> struct DefaultComp {
  inline bool operator()(V const &a, V const &b) { return a < b; }
};

template <class V, class Comp = DefaultComp<V>> class DefaultSwap {
public:
  inline DefaultSwap(V &a, V &b) {
    Comp c{};
    auto new_a = c(b, a) ? b : a;
    b = c(b, a) ? a : b;
    a = new_a;
  }
};

template <class V> class MinMaxSwap {
public:
  inline MinMaxSwap(V &a, V &b) {
    auto new_a = min(a, b);
    b = max(a, b);
    a = new_a;
  }
};

} // end of namespace swappers

template <int N> class SortingNetwork {
public:
  template <typename C> inline void operator()(C &container) {
    Sort<C, swappers::DefaultSwap<typename C::value_type>, 0, 1, N> sort(
        container);
  }

  template <typename C> inline void operator()(C *data) {
    Sort<C *, swappers::DefaultSwap<C>, 0, 1, N> sort(data);
  }

#ifdef NETWORK_SORT_SIMD_AVAILABLE
  class SIMD8 {
  public:
    inline void operator()(int *container) {
      std::array<Vec8i, N> block;
      for (int i = 0; i < N; ++i)
        block[i] = gather8i<0, N, 2 * N, 3 * N, 4 * N, 5 * N, 6 * N, 7 * N>(
            &container[i]);
      Sort<decltype(block), swappers::MinMaxSwap<Vec8i>, 0, 1, N> s(block);
      for (int i = 0; i < N; ++i)
        scatter<0, N, 2 * N, 3 * N, 4 * N, 5 * N, 6 * N, 7 * N>(block[i],
                                                                &container[i]);
    }
  };
#endif

  template <typename C, typename Swap, int Off, int Str, int A> struct Sort {
    inline Sort(C &container) {
      static_assert(A >= 3,
                    "This code path should not be sorting too small blocks");
      constexpr int A1 = size_of_first_block(A);
      constexpr int A2 = A - A1;
      Sort<C, Swap, Off, Str, A1> sort_block1(container);
      if (A2 > 1)
        Sort<C, Swap, Off + A1 * Str, Str, A2> sort_block2(container);
      Merge<C, Swap, Off, Str, A1, A2> merge_blocks(container);
    }
  };

  template <typename C, typename Swap, int Off, int Str>
  struct Sort<C, Swap, Off, Str, 2> {
    inline Sort(C &container) { Swap s(container[Off], container[Off + Str]); }
  };

  template <typename C, typename Swap, int Off, int Str>
  struct Sort<C, Swap, Off, Str, 1> {
    inline Sort(C &container) {}
  };

private:
  template <typename C, typename Swap, int Off, int Str, int A, int B>
  struct Merge {
    inline Merge(C &container) {
      static_assert(A % 2 == 0, "A must be a power of 2");
      static_assert(A <= 2 * B && B <= 2 * A, "A and B must be balanced.");

      Merge<C, Swap, Off, 2 * Str, A / 2, (B + 1) / 2> merge_odd_values(
          container);
      Merge<C, Swap, Off + Str, 2 * Str, A / 2, B / 2> merge_even_values(
          container);

      Finalise<C, Swap, Off + Str, Str, (A + B - 1) / 2> f(container);
    }
  };

  template <typename C, typename Swap, int Off, int Str>
  struct Merge<C, Swap, Off, Str, 1, 1> {
    inline Merge(C &container) { Swap s(container[Off], container[Off + Str]); }
  };

  template <typename C, typename Swap, int Off, int Str>
  struct Merge<C, Swap, Off, Str, 1, 2> {
    inline Merge(C &container) {
      Swap s1(container[Off], container[Off + 2 * Str]);
      Swap s2(container[Off], container[Off + Str]);
    }
  };

  template <typename C, typename Swap, int Off, int Str>
  struct Merge<C, Swap, Off, Str, 2, 1> {
    inline Merge(C &container) {
      Swap s1(container[Off], container[Off + 2 * Str]);
      Swap s2(container[Off + Str], container[Off + 2 * Str]);
    }
  };

  template <typename C, typename Swap, int Off, int Str, int A>
  struct Finalise {
    inline Finalise(C &container) {
      Swap s(container[Off], container[Off + Str]);
      Finalise<C, Swap, Off + 2 * Str, Str, A - 1> f(container);
    }
  };

  template <typename C, typename Swap, int Off, int Str>
  struct Finalise<C, Swap, Off, Str, 0> {
    inline Finalise(C &container) {}
  };

};

template <class C> inline void network_sort(C &S) {
  SortingNetwork<std::tuple_size<C>::value> sort(S);
}

namespace io_details {
// A dummy container that when subscripted returns a struct containing that
// subscript and an output stream
template <class OS> class OutputContainerPassthrough {
public:
  struct Lane {
    unsigned int place;
    unsigned int num_lanes;
    OS &output_str;
  };

  OutputContainerPassthrough(OS &os, unsigned int num_lanes)
      : m_os{os}, m_num_lanes{num_lanes} {}
  Lane operator[](unsigned int a) { return Lane{a, m_num_lanes, m_os}; }

private:
  OS &m_os;
  unsigned int m_num_lanes;
};

// Does not perform swapping, but instead outputs characters representing the
// swap to an output stream
template <class OS> class OutputSwap {
public:
  using Lane = typename OutputContainerPassthrough<OS>::Lane;
  OutputSwap(Lane a, Lane b) {
    const char end_marker = 'o';
    //const char empty_lane = '\372'; // centred dot
    const char empty_lane = '.';
    const char crossed_lane = '-';
    for (unsigned int i = 0; i < a.place; ++i)
      a.output_str << empty_lane << ' ';
    a.output_str << end_marker;
    for (unsigned int i = 0; i < (b.place - a.place) - 1; ++i)
      a.output_str << '-' << crossed_lane;
    a.output_str << '-' << end_marker;
    for (unsigned int i = 0; i < a.num_lanes - b.place - 1; ++i)
      a.output_str << ' ' << empty_lane;
    a.output_str << '\n';
  }
};

} // end of namespace io_details

template <class OS, int N> OS &operator<<(OS &os, SortingNetwork<N>) {
  io_details::OutputContainerPassthrough<OS> ocp(os, N);
  typename SortingNetwork<N>::template Sort<decltype(ocp),
                                            io_details::OutputSwap<OS>, 0, 1, N>
      sort(ocp);
  return os;
}

template<class C, class Swap>
inline void merge_network(C& container, unsigned int a, unsigned int b, unsigned int offset, unsigned int stride) {
  // Start by dealing with base cases
  if((a == 1) && (b == 1)) {
    Swap c(container[offset], container[offset + stride]);
    return;
  }
  if((a == 1) && (b == 2)) {
    Swap c1(container[offset], container[offset + 2 * stride]);
    Swap c2(container[offset], container[offset + stride]);
    return;
  }
  if((a == 2) && (b == 1)) {
    Swap c1(container[offset], container[offset + 2 * stride]);
    Swap c2(container[offset + stride], container[offset + 2 * stride]);
    return;
  }
     
  merge_network<C, Swap>(container, a / 2, (b + 1) / 2, offset, 2 * stride);
  merge_network<C, Swap>(container, a / 2, b / 2, offset + stride, 2 * stride);

  for(unsigned int i = 0; i < (a + b - 1) / 2; ++i)
    Swap c(container[offset + (2 * i + 1) * stride], container[offset + (2 * i + 2) * stride]);
}

template<class C, class Swap>
inline void sorting_network(C& container, unsigned int a, unsigned int offset, unsigned int stride) {

  if(a == 1)
    return;
  if(a == 2) {
    Swap s(container[offset], container[offset + stride]);
    return;
  }

  unsigned int const a1 = size_of_first_block(a);
  unsigned int const a2 = a - a1;

  sorting_network<C, Swap>(container, a1, offset, stride);
  sorting_network<C, Swap>(container, a2, offset + a1 * stride, stride);

  merge_network<C, Swap>(container, a1, a2, offset, stride);
}

} // end of namespace sorting_networks
