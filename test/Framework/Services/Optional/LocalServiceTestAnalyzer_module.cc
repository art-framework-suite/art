////////////////////////////////////////////////////////////////////////
// Class:       LocalServiceTestAnalyzer
// Module Type: analyzer
// File:        LocalServiceTestAnalyzer_module.cc
//
// Generated at Tue Oct 23 15:18:45 2012 by Christopher Green using artmod
// from art v1_02_04.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "test/Framework/Services/Interfaces/LocalServiceTestInterface.h"
#include "test/Framework/Services/Optional/LocalServiceTest.h"
#include "test/Framework/Services/Optional/LocalServiceTestInterfaceImpl.h"

namespace arttest {
  class LocalServiceTestAnalyzer;
}

class arttest::LocalServiceTestAnalyzer final : public art::EDAnalyzer {
public:
  explicit LocalServiceTestAnalyzer(fhicl::ParameterSet const & p);

  void analyze(art::Event const & e) override;

private:

};


arttest::LocalServiceTestAnalyzer::LocalServiceTestAnalyzer(fhicl::ParameterSet const &)
{
  art::ScheduleID const id(art::ScheduleID::first());
  (void) art::ServiceHandle<arttest::LocalServiceTest>(id)->schedule();
  (void) art::ServiceHandle<arttest::LocalServiceTestInterface>(id)->schedule();
  (void) art::ServiceHandle<arttest::LocalServiceTestInterfaceImpl>(id)->schedule();
}

void arttest::LocalServiceTestAnalyzer::analyze(art::Event const &)
{
  // Implementation of required member function here.
}

DEFINE_ART_MODULE(arttest::LocalServiceTestAnalyzer)
