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
    using SharedConstGroupPtr = Principal::SharedConstGroupPtr;
    static constexpr BranchType branch_type = Auxiliary::branch_type;

    EventPrincipal(EventAuxiliary const& aux,
                   ProcessConfiguration const& pc,
                   std::shared_ptr<History> history = std::make_shared<History>(),
                   std::unique_ptr<BranchMapper>&& mapper = std::make_unique<BranchMapper>(),
                   std::unique_ptr<DelayedReader>&& rtrv = std::make_unique<NoDelayedReader>(),
                   bool lastInSubRun = false,
                   int idx = 0,
                   EventPrincipal* = nullptr);

    // use compiler-generated copy c'tor, copy assignment.

    SubRunPrincipal const& subRunPrincipal() const;
    SubRunPrincipal& subRunPrincipal();

    std::shared_ptr<SubRunPrincipal> subRunPrincipalSharedPtr() { return subRunPrincipal_; }
    void setSubRunPrincipal(std::shared_ptr<SubRunPrincipal> srp) { subRunPrincipal_ = srp;  }

    EventID const& id() const { return aux().id(); }
    Timestamp const& time() const { return aux().time(); }
    bool isReal() const { return aux().isRealData(); }

    EventAuxiliary::ExperimentType ExperimentType() const { return aux().experimentType(); }

    EventAuxiliary const& aux() const { return aux_; }
    SubRunNumber_t subRun() const { return aux().subRun(); }
    RunNumber_t run() const { return id().run(); }

    RunPrincipal const& runPrincipal() const;
    RunPrincipal& runPrincipal();

    void addOnDemandGroup(BranchDescription const& desc,
                          cet::exempt_ptr<Worker> worker);

    EventSelectionIDVector const& eventSelectionIDs() const;

    History const&  history() const { return *history_; }
    History& history() { return *history_; }

    using Principal::getGroup;

    GroupQueryResult getGroup(ProductID const& pid) const;
    GroupQueryResult getByProductID(ProductID const& pid) const;

    void put(std::unique_ptr<EDProduct>&& edp,
             BranchDescription const& bd,
             std::unique_ptr<ProductProvenance const>&& productProvenance);

    void addGroup(BranchDescription const&);
    void addGroup(std::unique_ptr<EDProduct>&&, BranchDescription const&);

    ProductID branchIDToProductID(BranchID const bid) const;

    BranchType branchType() const override { return branch_type; }

    bool isLastInSubRun() const { return lastInSubRun_; }
    EDProductGetter const* productGetter(ProductID const& pid) const;

  private:

    BranchID productIDToBranchID(ProductID const& pid) const;

    void addOrReplaceGroup(std::unique_ptr<Group>&& g) override;

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

    // This function and its associated member datum are required to
    // handle the lifetime of a deferred getter, which in turn is required
    // because a group does not exist until it is placed in the event.
    EDProductGetter const* deferredGetter_(ProductID const& pid) const;

    EDProductGetter const* getEDProductGetterImpl(ProductID const& pid) const override {
      return getGroup(pid).result().get();
    }

  private:

    mutable
    std::map<ProductID, std::shared_ptr<DeferredProductGetter const>>
    deferredGetters_ {};

    EventAuxiliary aux_;

    std::shared_ptr<SubRunPrincipal> subRunPrincipal_ {nullptr};
    std::shared_ptr<History> history_;
    std::map<BranchListIndex, ProcessIndex> branchToProductIDHelper_ {};
    bool lastInSubRun_ {false};
  };

  inline
  bool
  isSameEvent(EventPrincipal const& a, EventPrincipal const& b)
  {
    return a.aux() == b.aux();
  }

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_Principal_EventPrincipal_h */
