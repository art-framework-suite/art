#include "art/Framework/EventProcessor/EvProcInitHelper.h"

#include "art/Persistency/Provenance/BranchIDListHelper.h"

using fhicl::ParameterSet;

art::EvProcInitHelper::EvProcInitHelper(ParameterSet const & ps)
  :
  servicesPS_(ps.get<ParameterSet>("services", ParameterSet())),
  schedulerPS_(servicesPS_.get<ParameterSet>("scheduler", ParameterSet()))
{
  // The BranchIDListRegistry is an indexed registry, and is a
  //  singleton. It must be cleared here because some processes run
  //  multiple EventProcessors in succession.
  BranchIDListHelper::clearRegistries();
}
