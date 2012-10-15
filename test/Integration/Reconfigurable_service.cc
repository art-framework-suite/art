// ======================================================================
//
// ======================================================================
#include "test/Integration/Reconfigurable.h"

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <iostream>

// ----------------------------------------------------------------------

Reconfigurable::Reconfigurable(fhicl::ParameterSet const & pset,
                               art::ActivityRegistry & r):
  debug_level_(pset.get<int>("debug_level",0)),
  other_value_(pset.get<int>("other_value",1)),
  postBeginJobCalled_(false)
{
  // r.watchPostProcessEvent(this, &Reconfigurable::throwError);
  r.watchPostBeginJob(this, &Reconfigurable::postBeginJob);
  mf::LogInfo("testing") << "Reconfigurable service created";
}


void Reconfigurable::reconfigure(fhicl::ParameterSet const& pset)
{
  std::cerr << "Reconfigurable service reconfiguring\n";

  debug_level_ = pset.get<int>("debug_level",0);
  // other_value_ = pset.get<int>("other_value",1);
}

void Reconfigurable::postBeginJob()
{
  postBeginJobCalled_ = true;
}

// ======================================================================
DEFINE_ART_SERVICE(Reconfigurable)
// ======================================================================


