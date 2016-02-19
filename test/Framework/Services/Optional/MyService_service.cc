// ======================================================================
//
// MyService
//
// ======================================================================

#include "test/Framework/Services/Optional/MyService.h"

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <iomanip>
#include <string>
#include <vector>

using arttest::MyService;
using arttest::MyServiceInterface;
using namespace cet;
using namespace fhicl;
using namespace std;

// ----------------------------------------------------------------------

static ParameterSet empty_pset;

// ----------------------------------------------------------------------

MyService::MyService(ParameterSet const & pset, art::ActivityRegistry&)
  : MyServiceInterface()
{
  mf::LogVerbatim("DEBUG") << "-----> Begin MyService::MyService(ParameterSet const & pset, art::ActivityRegistry&)";
  //ParameterSet services = pset.get<ParameterSet>("services", empty_pset);
  string val = pset.to_indented_string();
  mf::LogVerbatim("DEBUG") << "Contents of parameter set:";
  mf::LogVerbatim("DEBUG") << "";
  mf::LogVerbatim("DEBUG") << val;
  vector<string> keys = pset.get_pset_names();
  for (vector<string>::iterator I = keys.begin(), E = keys.end();
      I != E; ++I) {
    mf::LogVerbatim("DEBUG") << "key: " << *I;
  }
  mf::LogVerbatim("DEBUG") << "this: 0x" << std::hex << this << std::dec;
  mf::LogVerbatim("DEBUG") << "-----> End   MyService::MyService(ParameterSet const & pset, art::ActivityRegistry&)";
}  // c'tor

// ======================================================================

DEFINE_ART_SERVICE_INTERFACE_IMPL(arttest::MyService, arttest::MyServiceInterface)

// ======================================================================
