#ifndef test_TestObjects_AssnTestData_h
#define test_TestObjects_AssnTestData_h

#include "cpp0x/cstddef"
#include <string>

namespace arttest {
  struct AssnTestData;
}

struct arttest::AssnTestData {
  AssnTestData() : d1(0), d2(0), label() {}
  AssnTestData(size_t d1, size_t d2, std::string const &label)
    : d1(d1), d2(d2), label(label) {}
  size_t d1;
  size_t d2;
  std::string label;
};
#endif /* test_TestObjects_AssnTestData_h */

// Local Variables:
// mode: c++
// End:
