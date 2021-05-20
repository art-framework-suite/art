////////////////////////////////////////////////////////////////////////
// Class:       PMTestOutput
// Module Type: output
// File:        PMTestOutput_module.cc
//
// Generated at Tue Jun 18 09:47:06 2013 by Christopher Green using artmod
// from cetpkgsupport v1_02_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/OutputModule.h"
#include "art/Framework/Principal/fwd.h"
#include "fhiclcpp/types/ConfigurationTable.h"

namespace art::test {
  class PMTestOutput : public OutputModule {
  public:
    struct Config {
      fhicl::TableFragment<OutputModule::Config> omConfig;
    };

    using Parameters =
      fhicl::WrappedTable<Config, OutputModule::Config::KeysToIgnore>;
    explicit PMTestOutput(Parameters const& p);

  private:
    void
    write(art::EventPrincipal&) override
    {}
    void
    writeRun(art::RunPrincipal&) override
    {}
    void
    writeSubRun(art::SubRunPrincipal&) override
    {}
  };
}

art::test::PMTestOutput::PMTestOutput(Parameters const& p)
  : OutputModule(p().omConfig, p.get_PSet())
{}

DEFINE_ART_MODULE(art::test::PMTestOutput)
