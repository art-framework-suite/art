#ifndef art_test_Integration_ReadNextArbitrator_h
#define art_test_Integration_ReadNextArbitrator_h

#include "fhiclcpp/ParameterSet.h"

namespace arttest {
  class ReadNextArbitrator {
  public:
    ReadNextArbitrator(fhicl::ParameterSet const& ps)
      : threshold_{ps.get<unsigned>("threshold", 5)}
    {}
    auto
    threshold() const
    {
      return threshold_;
    }

  private:
    unsigned threshold_;
  };
}

#endif /* art_test_Integration_ReadNextArbitrator_h */

// Local variables:
// mode: c++
// End:
