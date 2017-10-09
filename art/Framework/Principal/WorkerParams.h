#ifndef art_Framework_Principal_WorkerParams_h
#define art_Framework_Principal_WorkerParams_h

// This struct is used to communicate parameters into the worker factory.

#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "fhiclcpp/ParameterSet.h"
#include <string>

namespace art {
  class ActionTable;
  class MasterProductRegistry;
  struct WorkerParams;
}

struct art::WorkerParams {
  WorkerParams(fhicl::ParameterSet const& procPset,
               fhicl::ParameterSet const& pset,
               MasterProductRegistry& reg,
               ProductDescriptions& producedProducts,
               ActionTable& actions,
               std::string const& processName);

  fhicl::ParameterSet const& procPset_;
  fhicl::ParameterSet const pset_;
  MasterProductRegistry& reg_;
  ProductDescriptions& producedProducts_;
  ActionTable& actions_;
  std::string const processName_;
};

inline art::WorkerParams::WorkerParams(fhicl::ParameterSet const& procPset,
                                       fhicl::ParameterSet const& pset,
                                       MasterProductRegistry& reg,
                                       ProductDescriptions& producedProducts,
                                       ActionTable& actions,
                                       std::string const& processName)
  : procPset_{procPset}
  , pset_{pset}
  , reg_{reg}
  , producedProducts_{producedProducts}
  , actions_{actions}
  , processName_{processName}
{}

#endif /* art_Framework_Principal_WorkerParams_h */

// Local Variables:
// mode: c++
// End:
