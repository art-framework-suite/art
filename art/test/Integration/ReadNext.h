#ifndef art_test_Integration_ReadNext_h
#define art_test_Integration_ReadNext_h

#include "fhiclcpp/ParameterSet.h"

namespace arttest {
  class ReadNextArbitrator {
  public:
    ReadNextArbitrator(fhicl::ParameterSet const& ps) : threshold_{ps.get<int>("threshold"), 5} {}
    auto threshold() const { return threshold_; }
  private:
    int threshold_;
  };
}

#endif

// Local variables:
// mode: c++
// End:
