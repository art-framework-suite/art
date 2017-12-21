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
              cet::exempt_ptr<Consumer> consumer,
              RangeSet const& rs)
  : DataViewImpl{rp, md, InRun, false, consumer}
  , principal_{rp}
  , aux_{rp.aux()}
  , productRangeSet_{rs}
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
art::Run::commit(RunPrincipal& rp,
                 bool const checkProducts,
                 std::set<TypeLabel> const& expectedProducts)
{
  // Check addresses only since type of 'rp' will hopefully change to
  // Principal&.
  assert(&rp == &principal_);
  checkPutProducts(checkProducts, expectedProducts, putProducts());
  commit(rp);
}

void
art::Run::commit(RunPrincipal& rp)
{
  for (auto& elem : putProducts()) {
    auto const& pd = elem.second.pd;
    auto productProvenancePtr = std::make_unique<ProductProvenance const>(
      pd.productID(), productstatus::present());
    rp.put(std::move(elem.second.prod),
           pd,
           std::move(productProvenancePtr),
           std::move(elem.second.rs));
  }

  // the cleanup is all or none
  putProducts().clear();
}
