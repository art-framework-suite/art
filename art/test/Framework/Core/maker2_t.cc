#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/CurrentProcessingContext.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/detail/ModuleFactory.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Version/GetReleaseVersion.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "fhiclcpp/ParameterSet.h"

using namespace art;
using fhicl::ParameterSet;

namespace
{
  ModuleDescription createModuleDescription(WorkerParams const& p)
  {
    ParameterSet const& procParams = p.procPset_;
    ParameterSet const& conf = p.pset_;
    return ModuleDescription(conf.id(),
                             conf.get<std::string>("module_type"),
                             conf.get<std::string>("module_label"),
                             ProcessConfiguration(p.processName_,
                                                  procParams.id(),
                                                  getReleaseVersion()));
  }
} // namespace

int main()
{
  ParameterSet p1;
  p1.put("module_type",std::string{"TestMod"});
  p1.put("module_label",std::string{"t1"});

  ParameterSet p2;
  p2.put("module_type",std::string{"TestMod"});
  p2.put("module_label",std::string{"t2"});

  art::ActionTable table;

  art::MasterProductRegistry preg;
  art::ProductDescriptions producedProducts;
  art::WorkerParams const params1{p1, p1, preg, producedProducts, table, "PROD"};
  art::WorkerParams const params2{p2, p2, preg, producedProducts, table, "PROD"};

  detail::ModuleFactory fact;
  auto w1 = fact.makeWorker(params1, createModuleDescription(params1));
  auto w2 = fact.makeWorker(params2, createModuleDescription(params2));

  return 0;
}
