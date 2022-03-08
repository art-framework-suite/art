////////////////////////////////////////////////////////////////////////
// Class:       PMTestFilter
// Module Type: filter
// File:        PMTestFilter_module.cc
//
// Generated at Mon Jun 17 16:44:46 2013 by Christopher Green using artmod
// from cetpkgsupport v1_02_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Principal/fwd.h"
#include "fhiclcpp/fwd.h"

namespace arttest {
  class PMTestFilter;
}

class arttest::PMTestFilter : public art::EDFilter {
public:
  explicit PMTestFilter(fhicl::ParameterSet const& p);

private:
  bool
  filter(art::Event&) override
  {
    return true;
  }
};

arttest::PMTestFilter::PMTestFilter(fhicl::ParameterSet const& p) : EDFilter{p}
{}

DEFINE_ART_MODULE(arttest::PMTestFilter)
