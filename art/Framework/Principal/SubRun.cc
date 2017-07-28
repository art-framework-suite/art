#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/Run.h"
#include "canvas/Persistency/Provenance/BranchType.h"

namespace art {

  namespace {
    Run*
    newRun(SubRunPrincipal const& srp,
           ModuleDescription const& md,
           cet::exempt_ptr<Consumer> consumer)
    {
      return srp.runPrincipalExemptPtr() ? new Run{srp.runPrincipal(), md, consumer} : nullptr;
    }
  }

  SubRun::SubRun(SubRunPrincipal const& srp,
                 ModuleDescription const& md,
                 cet::exempt_ptr<Consumer> consumer,
                 RangeSet const& rs) :
    DataViewImpl{srp, md, InSubRun, false, consumer},
    principal_{srp},
    aux_{srp.aux()},
    run_{newRun(srp, md, consumer)},
    productRangeSet_{rs}
  {}

  Run const&
  SubRun::getRun() const {
    if (!run_) {
      throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL run.\n";
    }
    return *run_;
  }

  EDProductGetter const*
  SubRun::productGetter(ProductID const pid) const
  {
    return principal_.productGetter(pid);
  }

  void
  SubRun::commit_(SubRunPrincipal& srp)
  {
    for (auto& elem : putProducts()) {
      auto const& pd = elem.second.pd;
      auto productProvenancePtr = std::make_unique<ProductProvenance const>(pd.productID(),
                                                                            productstatus::present());

      srp.put(std::move(elem.second.prod),
              pd,
              std::move(productProvenancePtr),
              std::move(elem.second.rs));
    }

    // the cleanup is all or none
    putProducts().clear();
  }

}
