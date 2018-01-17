#ifndef art_test_Integration_fastclonefail_v10_ClonedProd_h
#define art_test_Integration_fastclonefail_v10_ClonedProd_h

#include "canvas/Persistency/Common/Wrapper.h"

namespace arttest {

  class ClonedProd {
  public:
    ~ClonedProd();
    ClonedProd();

  private:
    int dp1_
#ifdef __clang__
      [[gnu::unused]]
#endif
      ;
  };

} // namespace arttest

#endif /* art_test_Integration_fastclonefail_v10_ClonedProd_h */

// Local Variables:
// mode: c++
// End:
