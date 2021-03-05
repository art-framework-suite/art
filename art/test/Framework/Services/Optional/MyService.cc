#include "MyService.h"

#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

using art::test::MyService;
using art::test::MyServiceInterface;
using namespace fhicl;

MyService::MyService(ParameterSet const& pset)
{
  mf::LogVerbatim("DEBUG")
    << "Begin MyService::MyService(ParameterSet const& pset)";
  auto const val = pset.to_indented_string();
  mf::LogVerbatim("DEBUG") << "Contents of parameter set:";
  mf::LogVerbatim("DEBUG") << "";
  mf::LogVerbatim("DEBUG") << val;
  for (auto const& key : pset.get_pset_names()) {
    mf::LogVerbatim("DEBUG") << "key: " << key;
  }
}

