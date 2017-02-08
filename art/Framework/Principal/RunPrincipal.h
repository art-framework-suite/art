#ifndef art_Framework_Principal_RunPrincipal_h
#define art_Framework_Principal_RunPrincipal_h
// vim: set sw=2:

//
//  RunPrincipal
//
//  Manages per-run data products.
//
//  This is not visible to modules, instead they use the Run class,
//  which is a proxy for this class.
//

#include "art/Framework/Principal/NoDelayedReader.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Provenance/BranchMapper.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "cetlib/exempt_ptr.h"

#include <memory>
#include <vector>

namespace art {

  class RunPrincipal final : public Principal {
  public:

    using Auxiliary = RunAuxiliary;
    static constexpr BranchType branch_type = RunAuxiliary::branch_type;

  public:

    RunPrincipal(RunAuxiliary const&,
                 ProcessConfiguration const&,
                 std::unique_ptr<BranchMapper>&& = std::make_unique<BranchMapper>(),
                 std::unique_ptr<DelayedReader>&& = std::make_unique<NoDelayedReader>(),
                 int const idx = 0,
                 cet::exempt_ptr<RunPrincipal const> = nullptr);

    RunAuxiliary const& aux() const { return aux_; }

    RunNumber_t run() const { return aux().run(); }

    RunID const& id() const { return aux().id(); }

    Timestamp const& beginTime() const { return aux().beginTime(); }
    Timestamp const& endTime() const { return aux().endTime(); }

    void setEndTime(Timestamp const& time) { aux_.setEndTime(time); }

    BranchType branchType() const override { return branch_type; }

    void fillGroup(BranchDescription const&) override;

    void put(std::unique_ptr<EDProduct>&&,
             BranchDescription const&,
             std::unique_ptr<ProductProvenance const>&&,
             RangeSet&&);

    RangeSet seenRanges() const override { return seenRangeSet_; }
    void updateSeenRanges(RangeSet const& rs) { seenRangeSet_ = rs; }
    RangeSetHandler const& rangeSetHandler() const;
    RangeSetHandler& rangeSetHandler();

  private:

    ProcessHistoryID const& processHistoryID() const override;
    void setProcessHistoryID(ProcessHistoryID const&) override;

    RunAuxiliary aux_;
    RangeSet seenRangeSet_ {RangeSet::invalid()};

  };

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_Principal_RunPrincipal_h */
