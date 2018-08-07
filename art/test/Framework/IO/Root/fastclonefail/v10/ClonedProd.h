#ifndef art_test_Framework_IO_Root_fastclonefail_v10_ClonedProd_h
#define art_test_Framework_IO_Root_fastclonefail_v10_ClonedProd_h
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Common/Wrapper.h"

namespace arttest {

  class ClonedProd {
  public:
    ~ClonedProd() noexcept;
    ClonedProd() noexcept;

  public:
    int dp1_{3};
  };

} // namespace arttest

#endif /* art_test_Framework_IO_Root_fastclonefail_v10_ClonedProd_h */

// Local Variables:
// mode: c++
// End:
