////////////////////////////////////////////////////////////////////////
// Class:       MyServiceUser
// Module Type: analyzer
// File:        MyServiceUser_module.cc
//
// Generated at Thu Sep  6 07:28:25 2012 by Christopher Green using artmod
// from art v1_01_01.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "test/Framework/Services/Interfaces/MyServiceInterface.h"
#include "test/Framework/Services/Optional/MyService.h"

namespace arttest {
  class MyServiceUser;
}

class arttest::MyServiceUser : public art::EDAnalyzer {
public:
  explicit MyServiceUser(fhicl::ParameterSet const & p);

  void analyze(art::Event const & e) override;

};

arttest::MyServiceUser::MyServiceUser(fhicl::ParameterSet const &p)
  : art::EDAnalyzer(p)
{
  art::ServiceHandle<MyServiceInterface>();
  art::ServiceHandle<MyService>();
}

void arttest::MyServiceUser::analyze(art::Event const &)
{
  // Implementation of required member function here.
}

DEFINE_ART_MODULE(arttest::MyServiceUser)
