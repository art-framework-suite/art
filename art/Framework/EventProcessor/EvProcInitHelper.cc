#include "art/Framework/EventProcessor/EvProcInitHelper.h"

#include "art/Persistency/Provenance/BranchIDListHelper.h"

using fhicl::ParameterSet;

art::EvProcInitHelper::EvProcInitHelper(ParameterSet const & ps)
  : servicesPS_(ps.get<ParameterSet>("services", {}))
  , schedulerPS_(servicesPS_.get<ParameterSet>("scheduler", {}))
{}
