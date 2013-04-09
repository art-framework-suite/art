////////////////////////////////////////////////////////////////////////
// Class:       FlushingGeneratorTest
// Module Type: analyzer
// File:        FlushingGeneratorTest_module.cc
//
// Generated at Tue Apr  2 10:48:17 2013 by Christopher Green using artmod
// from cetpkgsupport v1_01_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/OutputModule.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "fhiclcpp/ParameterSet.h"

#include <cassert>

namespace arttest {
  class FlushingGeneratorTest;
}

class arttest::FlushingGeneratorTest : public art::OutputModule {
public:
  explicit FlushingGeneratorTest(fhicl::ParameterSet const & p);

  void write(art::EventPrincipal const & e) override;
  void writeSubRun(art::SubRunPrincipal const & sr) override;
  void writeRun(art::RunPrincipal const & r) override;

  void beginRun(art::RunPrincipal const & r) override;
  void beginSubRun(art::SubRunPrincipal const & sr) override;
  void endRun(art::RunPrincipal const & r) override;
  void endSubRun(art::SubRunPrincipal const & sr) override;

private:
};


arttest::FlushingGeneratorTest::FlushingGeneratorTest(fhicl::ParameterSet const & p)
  :
  OutputModule(p)
{
}

void arttest::FlushingGeneratorTest::write(art::EventPrincipal const & e)
{
  assert(!e.id().isFlush());
}

void arttest::FlushingGeneratorTest::writeSubRun(art::SubRunPrincipal const & sr)
{
  assert(!sr.id().isFlush());
}

void arttest::FlushingGeneratorTest::writeRun(art::RunPrincipal const & r)
{
  assert(!r.id().isFlush());
}

void arttest::FlushingGeneratorTest::beginRun(art::RunPrincipal const & r)
{
  assert(!r.id().isFlush());
}

void arttest::FlushingGeneratorTest::beginSubRun(art::SubRunPrincipal const & sr)
{
  assert(!sr.id().isFlush());
}

void arttest::FlushingGeneratorTest::endRun(art::RunPrincipal const & r)
{
  assert(!r.id().isFlush());
}

void arttest::FlushingGeneratorTest::endSubRun(art::SubRunPrincipal const & sr)
{
  assert(!sr.id().isFlush());
}

DEFINE_ART_MODULE(arttest::FlushingGeneratorTest)
