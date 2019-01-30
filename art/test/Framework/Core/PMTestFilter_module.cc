////////////////////////////////////////////////////////////////////////
// Class:       PMTestFilter
// Module Type: filter
// File:        PMTestFilter_module.cc
//
// Generated at Mon Jun 17 16:44:46 2013 by Christopher Green using artmod
// from cetpkgsupport v1_02_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>

namespace arttest {
  class PMTestFilter;
}

class arttest::PMTestFilter : public art::EDFilter {
public:
  explicit PMTestFilter(fhicl::ParameterSet const& p);

private:
  bool filter(art::Event& e) override;

  // Declare member data here.
};

arttest::PMTestFilter::PMTestFilter(fhicl::ParameterSet const& p) : EDFilter{p}
{
  // Call appropriate Produces<>() functions here.
}

bool
arttest::PMTestFilter::filter(art::Event& e)
{
  // Implementation of required member function here.
}

DEFINE_ART_MODULE(arttest::PMTestFilter)
