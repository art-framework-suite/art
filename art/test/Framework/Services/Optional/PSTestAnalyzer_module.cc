////////////////////////////////////////////////////////////////////////
// Class:       PSTestAnalyzer
// Module Type: analyzer
// File:        PSTestAnalyzer_module.cc
//
// Generated at Tue Oct 23 15:18:45 2012 by Christopher Green using artmod
// from art v1_02_04.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/test/Framework/Services/Interfaces/PSTestInterface.h"
#include "art/test/Framework/Services/Optional/PSTest.h"
#include "art/test/Framework/Services/Optional/PSTestInterfaceImpl.h"

namespace arttest {
  class PSTestAnalyzer;
}

class arttest::PSTestAnalyzer final : public art::EDAnalyzer {
public:
  explicit PSTestAnalyzer(fhicl::ParameterSet const& p);

  void analyze(art::Event const& e) override;

private:
};

arttest::PSTestAnalyzer::PSTestAnalyzer(fhicl::ParameterSet const& pset)
  : art::EDAnalyzer(pset)
{
  art::ScheduleID const id(art::ScheduleID::first());
  (void)art::ServiceHandle<arttest::PSTest>(id)->schedule();
  (void)art::ServiceHandle<arttest::PSTestInterface>(id)->schedule();
  (void)art::ServiceHandle<arttest::PSTestInterfaceImpl>(id)->schedule();
}

void
arttest::PSTestAnalyzer::analyze(art::Event const&)
{
  // Implementation of required member function here.
}

DEFINE_ART_MODULE(arttest::PSTestAnalyzer)
