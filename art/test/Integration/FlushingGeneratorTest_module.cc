////////////////////////////////////////////////////////////////////////
// Class:       FlushingGeneratorTest
// Module Type: analyzer
// File:        FlushingGeneratorTest_module.cc
//
// Generated at Tue Apr  2 10:48:17 2013 by Christopher Green using artmod
// from cetpkgsupport v1_01_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/OutputModule.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "fhiclcpp/types/ConfigurationTable.h"

#include <cassert>

namespace arttest {
  class FlushingGeneratorTest;
}

class arttest::FlushingGeneratorTest : public art::OutputModule {
public:
  struct Config {
    fhicl::TableFragment<art::OutputModule::Config> omConfig;
  };

  using Parameters =
    fhicl::WrappedTable<Config, art::OutputModule::Config::KeysToIgnore>;
  explicit FlushingGeneratorTest(Parameters const& p);

  void write(art::EventPrincipal& e) override;
  void writeSubRun(art::SubRunPrincipal& sr) override;
  void writeRun(art::RunPrincipal& r) override;

  void beginRun(art::RunPrincipal const& r) override;
  void beginSubRun(art::SubRunPrincipal const& sr) override;
  void endRun(art::RunPrincipal const& r) override;
  void endSubRun(art::SubRunPrincipal const& sr) override;

private:
};

arttest::FlushingGeneratorTest::FlushingGeneratorTest(
  arttest::FlushingGeneratorTest::Parameters const& p)
  : OutputModule(p().omConfig, p.get_PSet())
{}

void
arttest::FlushingGeneratorTest::write(art::EventPrincipal& e)
{
  assert(!e.eventID().isFlush());
}

void
arttest::FlushingGeneratorTest::writeSubRun(art::SubRunPrincipal& sr)
{
  assert(!sr.subRunID().isFlush());
}

void
arttest::FlushingGeneratorTest::writeRun(art::RunPrincipal& r)
{
  assert(!r.runID().isFlush());
}

void
arttest::FlushingGeneratorTest::beginRun(art::RunPrincipal const& r)
{
  assert(!r.runID().isFlush());
}

void
arttest::FlushingGeneratorTest::beginSubRun(art::SubRunPrincipal const& sr)
{
  assert(!sr.subRunID().isFlush());
}

void
arttest::FlushingGeneratorTest::endRun(art::RunPrincipal const& r)
{
  assert(!r.runID().isFlush());
}

void
arttest::FlushingGeneratorTest::endSubRun(art::SubRunPrincipal const& sr)
{
  assert(!sr.subRunID().isFlush());
}

DEFINE_ART_MODULE(arttest::FlushingGeneratorTest)
