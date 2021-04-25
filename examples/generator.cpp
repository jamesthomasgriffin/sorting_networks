#include <cstring>
#include <iostream>

#include "sorting_networks.h"

using namespace sorting_networks;

template <class OS> void output_graph(OS &ostr, unsigned int n) {
  using C = typename io_details::OutputContainerPassthrough<OS>;
  C dummy_container(ostr, n);
  sorting_network<C, io_details::OutputGraphSwap<OS>>(dummy_container, n, 0, 1);
}

template <class OS> void output_pseudocode(OS &ostr, unsigned int n) {
  using C = typename io_details::OutputContainerPassthrough<OS>;
  C dummy_container(ostr, n);
  sorting_network<C, io_details::OutputSwap<OS>>(dummy_container, n, 0, 1);
}

void print_usage() {

  std::cout << "Usage\n";
  std::cout << '\n';
  std::cout << "  sngenerator [-graph] <number of elements>\n";
  std::cout << '\n';
  std::cout << "Output pseudocode, or optionally a graph for a Batcher "
               "odd/even sorting network.\n";
  std::cout << '\n';
}

int main(int argc, char *argv[]) {

  if (argc <= 1 || argc > 3) {
    print_usage();
    return 1;
  }

  bool print_graph = false;
  if (argc == 3) {
    const char graph_cmd[] = "-graph";
    if (strcmp(graph_cmd, argv[1])) {
      print_usage();
      return 1;
    }
    print_graph = true;
  }
  int n = std::atoi(argv[argc - 1]);
  if (n < 1) {
    std::cerr << "Argument must be an integer greater than zero.\n";
    return 1;
  }

  unsigned int n_items = n;
  if (print_graph)
    output_graph(std::cout, n);
  else
    output_pseudocode(std::cout, n);
  return 0;
}
