#ifndef art_test_Integration_fastclonefail_v11_ClonedProd_h
#define art_test_Integration_fastclonefail_v11_ClonedProd_h
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Common/Wrapper.h"

namespace arttest {

  class ClonedProd {
  public:
    ~ClonedProd();
    ClonedProd();

  public:
    double dp1_{5.0};
  };

} // namespace arttest

#endif /* art_test_Integration_fastclonefail_v11_ClonedProd_h */

// Local Variables:
// mode: c++
// End:
