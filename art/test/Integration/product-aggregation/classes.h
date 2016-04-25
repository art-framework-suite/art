#include "canvas/Persistency/Common/Wrapper.h"
#include "art/test/Integration/product-aggregation/CalibConstants.h"
#include "art/test/Integration/product-aggregation/Geometry.h"
#include "art/test/Integration/product-aggregation/TrackEfficiency.h"

namespace blah {
  art::Wrapper<arttest::CalibConstants> dummy1;
  art::Wrapper<arttest::Geometry> dummy2;
  art::Wrapper<arttest::TrackEfficiency> dummy3;
}

// Local variables:
// mode: c++
// End:
