// ======================================================================
//
// MyServiceInterface
//
// ======================================================================

#include "art/Framework/Services/Optional/MyServiceInterface.h"

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <string>
#include <vector>

using art::MyServiceInterface;
using namespace cet;
using namespace fhicl;
using namespace std;

// ----------------------------------------------------------------------

MyServiceInterface::MyServiceInterface()
{
  mf::LogVerbatim("DEBUG") << "-----> Begin MyServiceInterface::MyServiceInterface()";
  //ParameterSet services = pset.get<ParameterSet>("services", empty_pset);
  //string val = services.to_indented_string();
  //mf::LogVerbatim("DEBUG") << "Contents of services key:";
  //mf::LogVerbatim("DEBUG") << "";
  //mf::LogVerbatim("DEBUG") << val;
  //vector<string> keys = services.get_pset_keys();
  //for (vector<string>::iterator I = keys.begin(), E = keys.end();
  //    I != E; ++I) {
  //  mf::LogVerbatim("DEBUG") << "key: " << *I;
  //}
  mf::LogVerbatim("DEBUG") << "-----> End   MyServiceInterface::MyServiceInterface()";
}  // c'tor

// ======================================================================

