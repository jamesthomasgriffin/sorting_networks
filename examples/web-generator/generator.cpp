#include <iostream>

#include "network_sort.h"

using namespace sorting_networks;

template<class OS> 
void output_network(OS& ostr, unsigned int n) {
  using C = typename io_details::OutputContainerPassthrough<OS>;
  C dummy_container(ostr, n);
  sorting_network<C, io_details::OutputSwap<OS>>(dummy_container, n, 0, 1);
}


int main() {
  unsigned int n = 1;
  std::cout << "Enter number of elements to sort: ";
  std::cin >> n;
  std::cout << '\n';
  if(n > 0)
    output_network(std::cout, n);
  return 0;
}
