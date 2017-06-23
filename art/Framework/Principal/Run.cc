#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"
#include "fhiclcpp/ParameterSetRegistry.h"

using fhicl::ParameterSet;
using fhicl::ParameterSetID;
using fhicl::ParameterSetRegistry;

art::Run::Run(RunPrincipal const& rp,
              ModuleDescription const& md,
              ConsumesRecorder& consumesRecorder,
              RangeSet const& rs) :
  DataViewImpl{rp, md, InRun, false, consumesRecorder},
  principal_{rp},
  aux_{rp.aux()},
  productRangeSet_{rs}
{}

art::EDProductGetter const*
art::Run::productGetter(ProductID const pid) const
{
  return principal_.productGetter(pid);
}

bool
art::Run::getProcessParameterSet(std::string const& /*processName*/,
                                 std::vector<ParameterSet>& /*psets*/) const
{
  // Unimplemented
  return false;
}

void
art::Run::commit_(RunPrincipal& rp)
{
  for (auto& elem : putProducts()) {
    auto const& bd = elem.second.bd;
    auto productProvenancePtr = std::make_unique<ProductProvenance const>(bd.productID(),
                                                                          productstatus::present());
    rp.put(std::move(elem.second.prod),
           bd,
           std::move(productProvenancePtr),
           std::move(elem.second.rs));
  }

  // the cleanup is all or none
  putProducts().clear();
}
