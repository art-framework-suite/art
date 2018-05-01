// ======================================================================
//
// Throwing is a service that throws an exception when the
// postProcessEvent signal is emitted.
//
// ======================================================================

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "fhiclcpp/ParameterSet.h"

#include "messagefacility/MessageLogger/MessageLogger.h"

// ----------------------------------------------------------------------

namespace arttest {
  class Throwing {
  public:
    Throwing(fhicl::ParameterSet const& cfg, art::ActivityRegistry& ar);

  private:
    void throwError(art::Event const& e, art::ScheduleID);
  };
} // namespace arttest

arttest::Throwing::Throwing(fhicl::ParameterSet const&,
                            art::ActivityRegistry& r)
{
  r.sPostProcessEvent.watch(this, &Throwing::throwError);
  mf::LogInfo("testing") << "Throwing service created";
}

void
arttest::Throwing::throwError(art::Event const&, art::ScheduleID)
{
  mf::LogInfo("testing") << "Throwing service about to throw";
  throw art::Exception(art::errors::ProductNotFound)
    << "Intentional exception from Throwing service";
}

// ======================================================================
// The DECLARE macro call should be moved to the header file, should you
// create one.
DECLARE_ART_SERVICE(arttest::Throwing, LEGACY)
DEFINE_ART_SERVICE(arttest::Throwing)
// ======================================================================
