#include "art/Framework/Principal/SubRun.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/Run.h"

namespace art {

  namespace {
    Run *
    newRun(SubRunPrincipal const& srp, ModuleDescription const& md) {
      return (srp.runPrincipalExemptPtr() ? new Run{srp.runPrincipal(), md} : nullptr);
    }
  }

  SubRun::SubRun(SubRunPrincipal const& srp, ModuleDescription const& md, RangeSet const& rs) :
    DataViewImpl{srp, md, InSubRun},
    aux_{srp.aux()},
    run_{newRun(srp, md)},
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

  void
  SubRun::commit_(SubRunPrincipal& srp)
  {
    auto put_in_principal = [&srp](auto& elem) {

      // set provenance
      auto const& bd = elem.second.bd;
      auto subRunProductProvenancePtr = std::make_unique<ProductProvenance const>(bd.branchID(),
                                                                                  productstatus::present());

      srp.put(std::move(elem.second.prod),
              bd,
              std::move(subRunProductProvenancePtr),
              std::move(elem.second.rs));
    };

    cet::for_all(putProducts(), put_in_principal);

    // the cleanup is all or none
    putProducts().clear();
  }

}
