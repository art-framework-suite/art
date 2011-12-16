#ifndef test_Utilities_openmpTestFunc_h
#define test_Utilities_openmpTestFunc_h
#include <cstddef>

extern "C" {
  size_t openmpTestFunc(size_t numLoops, size_t mult) __attribute__((noinline));
}
#endif /* test_Utilities_openmpTestFunc_h */

// Local Variables:
// mode: c++
// End:
