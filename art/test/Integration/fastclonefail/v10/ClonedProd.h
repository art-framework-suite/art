#ifndef art_test_Integration_fastclonefail_v10_ClonedProd_h
#define art_test_Integration_fastclonefail_v10_ClonedProd_h
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Common/Wrapper.h"

namespace arttest {

  class ClonedProd {
  public:
    ~ClonedProd();
    ClonedProd();

  public:
    int dp1_{3};
  };

} // namespace arttest

#endif /* art_test_Integration_fastclonefail_v10_ClonedProd_h */

// Local Variables:
// mode: c++
// End:
