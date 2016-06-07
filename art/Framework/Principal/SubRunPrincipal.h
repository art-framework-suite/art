#ifndef art_Framework_Principal_SubRunPrincipal_h
#define art_Framework_Principal_SubRunPrincipal_h
// vim: set sw=2:

//  SubRunPrincipal
//
//  Manages per-subRun data products.
//
//  This is not visible to modules, instead they use the SubRun class,
//  which is a proxy for this class.

#include "art/Framework/Principal/NoDelayedReader.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Provenance/BranchMapper.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/EventRange.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"
#include "cetlib/exempt_ptr.h"

#include <memory>
#include <vector>

namespace art {

  class SubRunPrincipal final : public Principal {
  public:

    using Auxiliary = SubRunAuxiliary;
    static constexpr BranchType branch_type = Auxiliary::branch_type;

    SubRunPrincipal(SubRunAuxiliary const&,
                    ProcessConfiguration const&,
                    std::unique_ptr<BranchMapper>&& = std::make_unique<BranchMapper>(),
                    std::unique_ptr<DelayedReader>&& = std::make_unique<NoDelayedReader>(),
                    int idx = 0,
                    SubRunPrincipal* = nullptr);

    RunPrincipal const& runPrincipal() const;
    RunPrincipal& runPrincipal();

    std::shared_ptr<RunPrincipal> runPrincipalSharedPtr() { return runPrincipal_; }
    void setRunPrincipal(std::shared_ptr<RunPrincipal> rp) { runPrincipal_ = rp; }

    SubRunAuxiliary const& aux() const { return aux_; }
    SubRunID id() const { return aux().id(); }
    RunNumber_t run() const { return aux().run(); }
    SubRunNumber_t subRun() const { return aux().subRun(); }
    Timestamp const& beginTime() const { return aux().beginTime(); }
    Timestamp const& endTime() const { return aux().endTime(); }

    void setEndTime(Timestamp const& time) { aux_.setEndTime(time); }

    void put(std::unique_ptr<EDProduct>&&,
             BranchDescription const&,
             std::unique_ptr<ProductProvenance const>&&,
             RangeSet&&);

    void addGroup(BranchDescription const&);

    RangeSet seenRanges() const override { return rangeSet_; }
    void updateSeenRanges(RangeSet const& rs) { rangeSet_ = rs; }

    BranchType branchType() const override { return branch_type; }

  private:

    void addGroup(std::unique_ptr<EDProduct>&&,
                  BranchDescription const&,
                  RangeSet&&);
    void addOrReplaceGroup(std::unique_ptr<Group>&& g) override;
    ProcessHistoryID const& processHistoryID() const override;
    void setProcessHistoryID(ProcessHistoryID const& phid) override;

    SubRunAuxiliary aux_;
    std::shared_ptr<RunPrincipal> runPrincipal_ {nullptr};
    RangeSet rangeSet_ {RangeSet::invalid()};
  };

} // namespace art

#endif /* art_Framework_Principal_SubRunPrincipal_h */

// Local Variables:
// mode: c++
// End:
