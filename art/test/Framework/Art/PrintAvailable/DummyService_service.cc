////////////////////////////////////////////////////////////////////////
// Class:       DummyService
// Plugin Type: service (art v2_06_03)
// File:        DummyService_service.cc
//
// Generated at Fri May 19 10:24:54 2017 by Kyle Knoepfel using cetskelgen
// from cetlib version v2_03_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Services/Registry/ServiceMacros.h"

namespace art {
  namespace test {
    class DummyService;
  }
} // namespace art

class art::test::DummyService {
public:
  struct Config {
  };
  using Parameters = art::ServiceTable<Config>;
  explicit DummyService(Parameters const&) {}
};

DECLARE_ART_SERVICE(art::test::DummyService, LEGACY)
DEFINE_ART_SERVICE(art::test::DummyService)
