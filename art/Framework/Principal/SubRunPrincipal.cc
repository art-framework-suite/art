#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/GroupFactory.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace art {

  SubRunPrincipal::SubRunPrincipal(SubRunAuxiliary const& aux,
                                   ProcessConfiguration const& pc,
                                   std::auto_ptr<BranchMapper> mapper,
                                   std::auto_ptr<DelayedReader> rtrv) :
    Principal(pc, aux.processHistoryID_, mapper, rtrv),
    runPrincipal_(),
    aux_(aux)
  {
    if (ProductMetaData::instance().productProduced(InSubRun)) {
      addToProcessHistory();
    }
  }

  void
  SubRunPrincipal::addOrReplaceGroup(std::auto_ptr<Group> g) {
    cet::exempt_ptr<Group const> group =
      getExistingGroup(g->productDescription().branchID());
    if (!group) {
      addGroup_(g);
    } else {
      BranchDescription const& bd = group->productDescription();
      mf::LogWarning("SubRunMerging")
        << "Problem found while adding product provenance, "
        << "product already exists for ("
        << bd.friendlyClassName() << ","
        << bd.moduleLabel() << ","
        << bd.productInstanceName() << ","
        << bd.processName()
        << ")\n";
    }
  }

  void
  SubRunPrincipal::addGroup(BranchDescription const& bd) {
    addOrReplaceGroup(gfactory::make_group(bd, ProductID()));
  }

  void
  SubRunPrincipal::addGroup(std::auto_ptr<EDProduct> prod,
                            BranchDescription const& bd) {
    addOrReplaceGroup(gfactory::make_group(prod, bd, ProductID()));
  }

  void
  SubRunPrincipal::put(std::auto_ptr<EDProduct> edp,
                       BranchDescription const& bd,
                       std::auto_ptr<ProductProvenance const> productProvenance) {

    if (edp.get() == 0) {
      throw art::Exception(art::errors::InsertFailure,"Null Pointer")
        << "put: Cannot put because auto_ptr to product is null."
        << "\n";
    }
    branchMapper().insert(productProvenance);
    this->addGroup(edp, bd);
  }

  RunPrincipal const& SubRunPrincipal::runPrincipal() const {
    if (!runPrincipal_) {
      throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL runPrincipal.\n";
    }
    return *runPrincipal_;
  }

  RunPrincipal & SubRunPrincipal::runPrincipal() {
    if (!runPrincipal_) {
      throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL runPrincipal.\n";
    }
    return *runPrincipal_;
  }

  void
  SubRunPrincipal::mergeSubRun(std::shared_ptr<SubRunPrincipal> srp) {
    if (srp.get() == this) return; // Nothing to do.

    aux_.mergeAuxiliary(srp->aux());

    for (Principal::const_iterator i = srp->begin(), iEnd = srp->end(); i != iEnd; ++i) {

      std::auto_ptr<Group> g(new Group());
      g->swap(*i->second);

      addOrReplaceGroup(g);
    }
  }
}

