#include "art/Framework/Art/detail/ServiceNames.h"

namespace {

  using ServiceNames_bimap_t = art::detail::ServiceNames::ServiceNames_bimap_t;

  auto
  initializeBimap()
  {
    using position = ServiceNames_bimap_t::value_type;
    ServiceNames_bimap_t result;
    result.insert(position{"floating_point_control", "FloatingPointControl"});
    return result;
  }
} // namespace

ServiceNames_bimap_t art::detail::ServiceNames::lookup_ = initializeBimap();
