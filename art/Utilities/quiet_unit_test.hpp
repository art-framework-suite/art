#ifndef art_Utilities_quiet_unit_test_hpp
#define art_Utilities_quiet_unit_test_hpp

// Use this header, rather than unit_test.hpp directly, to include the
// GCC pragma magic to silence warnings about overloaded virtual
// functions in boost/test/unit_test_log_formatter.hpp.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#if __APPLE__ && __MACH__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include "boost/test/included/unit_test.hpp"
#pragma GCC diagnostic pop

#endif /* art_Utilities_quiet_unit_test_hpp */
