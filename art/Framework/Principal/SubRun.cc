#include "art/Framework/Principal/SubRun.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/Run.h"

namespace art {

  namespace {
    Run *
    newRun(SubRunPrincipal& srp, ModuleDescription const& md) {
      return (srp.runPrincipalSharedPtr() ? new Run(srp.runPrincipal(), md) : nullptr);
    }
  }

  SubRun::SubRun(SubRunPrincipal& srp, ModuleDescription const& md) :
    DataViewImpl(srp, md, InSubRun),
    aux_(srp.aux()),
    run_(newRun(srp, md)),
    seenRanges_{srp.rangeSetHandler().seenRanges()}
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
  SubRun::commit_()
  {
    auto & srp = dynamic_cast<SubRunPrincipal &>(principal());
    auto put_in_principal = [&srp](auto& elem) {

      // set provenance
      auto subRunProductProvenancePtr = std::make_unique<ProductProvenance const>(elem.first,
                                                                                  productstatus::present());

      srp.put( std::move(elem.second.prod),
               elem.second.bd,
               std::move(subRunProductProvenancePtr),
               std::move(elem.second.rs));
    };

    cet::for_all( putProducts(), put_in_principal );

    // the cleanup is all or none
    putProducts().clear();
  }

}
