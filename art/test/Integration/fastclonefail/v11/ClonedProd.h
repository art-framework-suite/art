#ifndef art_test_Integration_fastclonefail_v11_ClonedProd_h
#define art_test_Integration_fastclonefail_v11_ClonedProd_h

#include "canvas/Persistency/Common/Wrapper.h"

namespace arttest {

  class ClonedProd {
  public:
    ~ClonedProd();
    ClonedProd();

  private:
    double dp1_;
  };

} // namespace arttest

#endif /* art_test_Integration_fastclonefail_v11_ClonedProd_h */

// Local Variables:
// mode: c++
// End:
