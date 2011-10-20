// ======================================================================
//
// ======================================================================

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "fhiclcpp/ParameterSet.h"
#include "test/Integration/Reconfigurable.h"

#include "messagefacility/MessageLogger/MessageLogger.h"

#include <iostream>

// ----------------------------------------------------------------------

Reconfigurable::Reconfigurable(fhicl::ParameterSet const & pset,
                               art::ActivityRegistry &):
  debug_level_(pset.get<int>("debug_level",0)),
  other_value_(pset.get<int>("other_value",1))
{
  // r.watchPostProcessEvent(this, &Reconfigurable::throwError);
  mf::LogInfo("testing") << "Reconfigurable service created";
}


void Reconfigurable::reconfigure(fhicl::ParameterSet const& pset)
{
  std::cerr << "Reconfigurable service reconfiguring\n";

  debug_level_ = pset.get<int>("debug_level",0);
  // other_value_ = pset.get<int>("other_value",1);
}

// ======================================================================
DEFINE_ART_SERVICE(Reconfigurable)
// ======================================================================


