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

namespace arttest
{
  class Throwing
  {
  public:
   Throwing(fhicl::ParameterSet const& cfg, art::ActivityRegistry& ar);

  private:
    void throwError(art::Event const& e);
  };
}

arttest::Throwing::Throwing(fhicl::ParameterSet const &,
                            art::ActivityRegistry & r)
{
  r.watchPostProcessEvent(this, &Throwing::throwError);
  mf::LogInfo("testing") << "Throwing service created";
}


void
arttest::Throwing::throwError(art::Event const&)
{
  mf::LogInfo("testing") << "Throwing service about to throw";
  throw art::Exception(art::errors::ProductNotFound)
    << "Intentional exception from Throwing service";
}

// ======================================================================
DEFINE_ART_SERVICE(arttest::Throwing)
// ======================================================================


