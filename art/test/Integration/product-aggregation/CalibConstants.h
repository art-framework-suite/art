#ifndef test_Integration_product_aggregation_CalibConstants_h
#define test_Integration_product_aggregation_CalibConstants_h

#include "canvas/Persistency/Provenance/IDNumber.h"

namespace arttest {

  class CalibConstants {
  public:
    explicit CalibConstants() = default;
    explicit CalibConstants(art::SubRunNumber_t const sr) : subrun_{sr} {}
    auto subrun() const { return subrun_; }
    void aggregate(CalibConstants const&){}
  private:
    art::SubRunNumber_t subrun_;
  };

}

#endif

// Local variables:
// mode: c++
// End:
