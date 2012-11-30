////////////////////////////////////////////////////////////////////////
// tbb_pfor_01_t
//
// Demonstrate trivial use of blocked_range and parallel_for over a
// vector.
//
////////////////////////////////////////////////////////////////////////
#include "tbb/tbb.h"

#include <iostream>

int main()
{
  size_t const n = 500000;
  std::vector<double> v(n, 27.64), vr;
  vr.reserve(n);
  typedef tbb::blocked_range<typename decltype(v)::const_iterator> br_t;
  tbb::parallel_for(br_t(v.cbegin(), v.cend()),
  [&v, &vr](br_t const & r) -> void {
  for (auto i : r) { vr.emplace_back(i * 2.0); }
  }
                   );
  std::cout << vr.back() << std::endl;
}
