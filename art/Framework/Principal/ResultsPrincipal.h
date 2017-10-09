#ifndef art_Framework_Principal_ResultsPrincipal_h
#define art_Framework_Principal_ResultsPrincipal_h
// vim: set sw=2:

//
//  ResultsPrincipal
//
//  Manages per-file results data products.
//
//  This is not visible to modules, instead they use the Results class,
//  which is a proxy for this class.
//

#include "art/Framework/Principal/NoDelayedReader.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Provenance/BranchMapper.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ResultsAuxiliary.h"
#include "cetlib/exempt_ptr.h"
#include <memory>
#include <vector>

namespace art {
  class ResultsPrincipal;
}

class art::ResultsPrincipal final : public Principal {
public:
  using Auxiliary = ResultsAuxiliary;
  static constexpr BranchType branch_type = ResultsAuxiliary::branch_type;

  ResultsPrincipal(
    ResultsAuxiliary const&,
    ProcessConfiguration const&,
    cet::exempt_ptr<ProductTable const> presentProducts,
    std::unique_ptr<BranchMapper>&& mapper = std::make_unique<BranchMapper>(),
    std::unique_ptr<DelayedReader>&& rtrv =
      std::make_unique<NoDelayedReader>());

  ResultsAuxiliary const&
  aux() const
  {
    return aux_;
  }

  void put(std::unique_ptr<EDProduct>&&,
           BranchDescription const&,
           std::unique_ptr<ProductProvenance const>&&);

  void fillGroup(BranchDescription const&) override;

  BranchType branchType() const override;
  RangeSet
  seenRanges() const override
  {
    return RangeSet::invalid();
  }

private:
  ProcessHistoryID const& processHistoryID() const override;

  void setProcessHistoryID(ProcessHistoryID const& phid) override;

  ResultsAuxiliary aux_;
};

#endif /* art_Framework_Principal_ResultsPrincipal_h */

// Local Variables:
// mode: c++
// End:
