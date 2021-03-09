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
#include "MyServiceInterface.h"
#include "MyService.h"

namespace art::test {
  class MyServiceUser;
}

class art::test::MyServiceUser : public EDAnalyzer {
public:
  explicit MyServiceUser(fhicl::ParameterSet const& p);

private:
  void
  analyze(art::Event const&) override
  {}
};

art::test::MyServiceUser::MyServiceUser(fhicl::ParameterSet const& p)
  : EDAnalyzer(p)
{
  ServiceHandle<MyServiceInterface>();
  ServiceHandle<MyService>();
}

DEFINE_ART_MODULE(art::test::MyServiceUser)
