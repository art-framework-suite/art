////////////////////////////////////////////////////////////////////////
// Class:       FlushingGeneratorTestFilter
// Module Type: analyzer
// File:        FlushingGeneratorTestFilter_module.cc
//
// Generated at Tue Apr  2 10:48:17 2013 by Christopher Green using artmod
// from cetpkgsupport v1_01_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "fhiclcpp/ParameterSet.h"

#include <cassert>

namespace arttest {
  class FlushingGeneratorTestFilter;
}

class arttest::FlushingGeneratorTestFilter : public art::EDFilter {
public:
  explicit FlushingGeneratorTestFilter(fhicl::ParameterSet const & p);

  bool filter(art::Event &) override;

  bool beginRun(art::Run & r) override;
  bool beginSubRun(art::SubRun & sr) override;
  bool endRun(art::Run & r, art::RangeSet const&) override;
  bool endSubRun(art::SubRun & sr, art::RangeSet const&) override;

private:
};


arttest::FlushingGeneratorTestFilter::FlushingGeneratorTestFilter(fhicl::ParameterSet const &)
{
}

bool arttest::FlushingGeneratorTestFilter::filter(art::Event & e)
{
  assert(!e.id().isFlush());
  return true;
}

bool arttest::FlushingGeneratorTestFilter::beginRun(art::Run & r)
{
  assert(!r.id().isFlush());
  return true;
}

bool arttest::FlushingGeneratorTestFilter::beginSubRun(art::SubRun & sr)
{
  assert(!sr.id().isFlush());
  return true;
}

bool arttest::FlushingGeneratorTestFilter::endRun(art::Run & r, art::RangeSet const&)
{
  assert(!r.id().isFlush());
  return true;
}

bool arttest::FlushingGeneratorTestFilter::endSubRun(art::SubRun & sr, art::RangeSet const&)
{
  assert(!sr.id().isFlush());
  return true;
}

DEFINE_ART_MODULE(arttest::FlushingGeneratorTestFilter)
