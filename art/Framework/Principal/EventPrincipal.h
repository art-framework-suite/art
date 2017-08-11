#ifndef art_Framework_Principal_EventPrincipal_h
#define art_Framework_Principal_EventPrincipal_h
// vim: set sw=2:

//  EventPrincipal
//
//  Manages per-event data products.
//
//  This is not visible to modules, instead they use the Event class,
//  which is a proxy for this class.

#include "art/Framework/Principal/NoDelayedReader.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "canvas/Persistency/Provenance/BranchMapper.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/History.h"
#include "cetlib/exempt_ptr.h"

#include <map>
#include <memory>
#include <vector>

namespace art {

  class EventID;

  class EventPrincipal final : public Principal {
  public:

    using Auxiliary = EventAuxiliary;
    static constexpr BranchType branch_type = Auxiliary::branch_type;

    EventPrincipal(EventAuxiliary const& aux,
                   ProcessConfiguration const& pc,
                   std::shared_ptr<History> history = std::make_shared<History>(),
                   std::unique_ptr<BranchMapper>&& mapper = std::make_unique<BranchMapper>(),
                   std::unique_ptr<DelayedReader>&& rtrv = std::make_unique<NoDelayedReader>(),
                   bool lastInSubRun = false);

    SubRunPrincipal const& subRunPrincipal() const;

    cet::exempt_ptr<SubRunPrincipal const> subRunPrincipalExemptPtr() const { return subRunPrincipal_; }
    void setSubRunPrincipal(cet::exempt_ptr<SubRunPrincipal const> srp) { subRunPrincipal_ = srp;  }

    EventID const& id() const { return aux().id(); }
    Timestamp const& time() const { return aux().time(); }
    bool isReal() const { return aux().isRealData(); }

    EventAuxiliary::ExperimentType ExperimentType() const { return aux().experimentType(); }

    EventAuxiliary const& aux() const { return aux_; }
    SubRunNumber_t subRun() const { return id().subRun(); }
    RunNumber_t run() const { return id().run(); }
    EventNumber_t event() const { return id().event(); }

    EventSelectionIDVector const& eventSelectionIDs() const;

    History const& history() const { return *history_; }

    using Principal::getGroup;

    void put(std::unique_ptr<EDProduct>&& edp,
             BranchDescription const& pd,
             std::unique_ptr<ProductProvenance const>&& productProvenance);

    void fillGroup(BranchDescription const&) override;

    BranchType branchType() const override { return branch_type; }

    bool isLastInSubRun() const { return lastInSubRun_; }
    RangeSet seenRanges() const override { return RangeSet::invalid(); }

  private:

    void throwIfExistingGroup(BranchDescription const& pd) const;

    ProcessHistoryID const&
    processHistoryID() const override
    {
      return history().processHistoryID();
    }

    void
    setProcessHistoryID(ProcessHistoryID const& phid) override
    {
      return history().setProcessHistoryID(phid);
    }

  private:

    EventAuxiliary aux_;

    cet::exempt_ptr<SubRunPrincipal const> subRunPrincipal_ {nullptr};
    std::shared_ptr<History> history_;
    bool lastInSubRun_ {false};
  };

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_Principal_EventPrincipal_h */
