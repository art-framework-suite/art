#ifndef test_Integration_product_aggregation_TrackEfficiency_h
#define test_Integration_product_aggregation_TrackEfficiency_h

#include "canvas/Persistency/Provenance/IDNumber.h"

namespace arttest {

  class TrackEfficiency {
  public:

    explicit TrackEfficiency() = default;
    explicit TrackEfficiency(unsigned const num,
                             unsigned const denom)
      : num_{num}, denom_{denom}
    {}

    auto efficiency() const
    {
      return static_cast<double>(num_)/denom_;
    }

    void aggregate(TrackEfficiency const& other)
    {
      num_ += other.num_;
      denom_ += other.denom_;
    }

  private:
    unsigned num_;
    unsigned denom_;
  };

}

#endif

// Local variables:
// mode: c++
// End:
