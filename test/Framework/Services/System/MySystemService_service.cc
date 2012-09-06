// ======================================================================
//
// MySystemService
//
// ======================================================================

#include "test/Framework/Services/System/MySystemService.h"

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

using art::MySystemService;
using namespace cet;
using namespace fhicl;
using namespace std;

// ----------------------------------------------------------------------

static ParameterSet empty_pset;

// ----------------------------------------------------------------------

MySystemService::MySystemService(ParameterSet const & pset,
                                 art::ActivityRegistry&)
{
  ParameterSet services = pset.get<ParameterSet>("services", empty_pset);
  string val = services.to_indented_string();
  mf::LogVerbatim("DEBUG") << "Contents of services key:";
  mf::LogVerbatim("DEBUG") << "";
  mf::LogVerbatim("DEBUG") << val;
  vector<string> keys = services.get_pset_keys();
  for (vector<string>::iterator I = keys.begin(), E = keys.end();
      I != E; ++I) {
    mf::LogVerbatim("DEBUG") << "key: " << *I;
  }
}  // c'tor

// ======================================================================

DEFINE_ART_SYSTEM_SERVICE(MySystemService)

// ======================================================================
