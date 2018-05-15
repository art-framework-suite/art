// ======================================================================
// Throwing is a service that throws an exception when the
// postProcessEvent signal is emitted.
// ======================================================================

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

using namespace art;

namespace arttest {
  class Throwing {
  public:
    struct Config {
    };
    using Parameters = ServiceTable<Config>;
    Throwing(Parameters const& cfg, ActivityRegistry& ar);

  private:
    void throwError(art::Event const& e, ScheduleContext);
  };
}

arttest::Throwing::Throwing(Parameters const&, ActivityRegistry& r)
{
  r.sPostProcessEvent.watch(this, &Throwing::throwError);
  mf::LogInfo("testing") << "Throwing service created";
}

void
arttest::Throwing::throwError(Event const&, ScheduleContext)
{
  mf::LogInfo("testing") << "Throwing service about to throw";
  throw Exception(errors::ProductNotFound)
    << "Intentional exception from Throwing service";
}

DECLARE_ART_SERVICE(arttest::Throwing, LEGACY)
DEFINE_ART_SERVICE(arttest::Throwing)
