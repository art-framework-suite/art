#ifndef art_test_Framework_Services_System_fpc_utils_h
#define art_test_Framework_Services_System_fpc_utils_h
// Define functions to allow reproduction of FPC errors.
// These functions must *not* be inlined or subject to LTO.
namespace arttest {
  double divit(double x, double y);
  double multit(double x, double y);
}
#endif /* art_test_Framework_Services_System_fpc_utils_h */

// Local Variables:
// mode: c++
// End:
