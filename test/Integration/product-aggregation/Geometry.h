#ifndef test_Integration_product_aggregation_Geometry_h
#define test_Integration_product_aggregation_Geometry_h

#include "canvas/Persistency/Provenance/IDNumber.h"

namespace arttest {

  class Geometry {
  public:
    explicit Geometry() = default;
    explicit Geometry(art::RunNumber_t const r) : run_{r} {}
    auto run() const { return run_; }
    void aggregate(Geometry const&){}
  private:
    art::RunNumber_t run_;
  };

}

#endif

// Local variables:
// mode: c++
// End:
