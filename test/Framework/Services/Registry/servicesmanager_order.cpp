
#include "FWCore/ServiceRegistry/test/stubs/DummyServiceE0.h"
#include "art/Framework/PluginManager/ProblemTracker.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceWrapper.h"
#include "art/Framework/Services/Registry/ServicesManager.h"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"
#include <cstdlib>
#include <iostream>
#include <vector>

//NOTE: I need to open a 'back door' so I can test ServiceManager 'inheritance'
#define private public
#include "art/Framework/Services/Registry/ServiceToken.h"
#undef private

int main()
{
  using namespace art::serviceregistry;

  // We must initialize the plug-in manager first
  art::AssertHandler ah;

  // These services check the order their constructor, postBeginJob,
  // postEndJob, and destructor are called and if they are not
  // called in the correct order they will abort
  typedef testserviceregistry::DummyServiceE0 Service0;
  typedef testserviceregistry::DummyServiceA1 Service1;
  typedef testserviceregistry::DummyServiceD2 Service2;
  typedef testserviceregistry::DummyServiceB3 Service3;
  typedef testserviceregistry::DummyServiceC4 Service4;

  // Build the services in a manner similar to the way the are constructed
  // in a fw job.  Build one service directly, then three based
  // on parameter sets, then another one directly.  ServiceB3
  // includes an explicit dependence on ServiceD2 so build on
  // demand is also tested.

  std::vector<fhicl::ParameterSet> vps;
  std::shared_ptr<ServicesManager> legacy(new ServicesManager(vps));

  art::ActivityRegistry ar;
  fhicl::ParameterSet pset;
  std::unique_ptr<Service0> s0(new Service0(pset, ar));
  std::shared_ptr<ServiceWrapper<Service0> >
      wrapper (new ServiceWrapper<Service0>(s0));
  legacy->put(wrapper);
  legacy->copySlotsFrom(ar);
  art::ServiceToken legacyToken(legacy);

  std::vector<fhicl::ParameterSet> vps1;

  fhicl::ParameterSet ps1;
  std::string typeName1("DummyServiceA1");
  ps1.addParameter("service_type", typeName1);
  vps1.push_back(ps1);

  // The next two are intentionally swapped to test build
  // on demand feature.  DummyServiceB3 depends on DummyServiceD2
  // so they should end up getting built in the reverse of the
  // order specified here.

  fhicl::ParameterSet ps3;
  std::string typeName3("DummyServiceB3");
  ps3.addParameter("service_type", typeName3);
  vps1.push_back(ps3);

  fhicl::ParameterSet ps2;
  std::string typeName2("DummyServiceD2");
  ps2.addParameter("service_type", typeName2);
  vps1.push_back(ps2);

  std::shared_ptr<ServicesManager> legacy2(new ServicesManager(legacyToken,
                                                                 kTokenOverrides,
                                                                 vps1));
  art::ServiceToken legacyToken2(legacy2);


  ServicesManager sm(legacyToken2, kOverlapIsError, vps);

  art::ActivityRegistry ar4;
  fhicl::ParameterSet pset4;
  std::unique_ptr<Service4> s4(new Service4(pset4, ar4));
  std::shared_ptr<ServiceWrapper<Service4> >
      wrapper4 (new ServiceWrapper<Service4>(s4));
  sm.put(wrapper4);
  sm.copySlotsFrom(ar4);


  art::ActivityRegistry actReg;
  sm.connectTo(actReg);
  actReg.postBeginJobSignal_();
  actReg.postEndJobSignal_();

  return 0;
}
