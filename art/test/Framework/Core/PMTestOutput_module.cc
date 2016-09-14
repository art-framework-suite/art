////////////////////////////////////////////////////////////////////////
// Class:       PMTestOutput
// Module Type: output
// File:        PMTestOutput_module.cc
//
// Generated at Tue Jun 18 09:47:06 2013 by Christopher Green using artmod
// from cetpkgsupport v1_02_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/OutputModule.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Services/System/FileCatalogMetadata.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Table.h"


namespace arttest {
  class PMTestOutput;
}

class arttest::PMTestOutput : public art::OutputModule {
public:

  struct Config {
    fhicl::TableFragment<art::OutputModule::Config> omConfig;
  };

  using Parameters = fhicl::Table<Config, art::OutputModule::Config::KeysToIgnore>;
  explicit PMTestOutput(Parameters const & p);
  virtual ~PMTestOutput();

  void write(art::EventPrincipal & e) override;
  void writeRun(art::RunPrincipal & r) override;
  void writeSubRun(art::SubRunPrincipal & sr) override;


private:

  // Declare member data here.

};


arttest::PMTestOutput::PMTestOutput(arttest::PMTestOutput::Parameters const & p)
 :
  OutputModule(p().omConfig, p.get_PSet()) // ,
 // More initializers here.
{}

arttest::PMTestOutput::~PMTestOutput()
{
  // Clean up dynamic memory and other resources here.
}

void arttest::PMTestOutput::write(art::EventPrincipal & e)
{
  // Implementation of required member function here.
}

void arttest::PMTestOutput::writeRun(art::RunPrincipal & r)
{
  // Implementation of required member function here.
}

void arttest::PMTestOutput::writeSubRun(art::SubRunPrincipal & sr)
{
  // Implementation of required member function here.
}

DEFINE_ART_MODULE(arttest::PMTestOutput)
