#ifndef art_Framework_Core_EventPrincipal_h
#define art_Framework_Core_EventPrincipal_h

/*----------------------------------------------------------------------

EventPrincipal: This is the class responsible for management of
per event EDProducts. It is not seen by reconstruction code;
such code sees the Event class, which is a proxy for EventPrincipal.

The major internal component of the EventPrincipal
is the DataBlock.

----------------------------------------------------------------------*/

#include "art/Framework/Core/Principal.h"
#include "art/Persistency/Common/EDProductGetter.h"
#include "art/Persistency/Provenance/BranchMapper.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/EventAuxiliary.h"
#include "art/Persistency/Provenance/History.h"
#include "cetlib/exempt_ptr.h"
#include "cpp0x/memory"
#include <vector>

namespace art {
  class EventID;
  class SubRunPrincipal;
  class RunPrincipal;
  class UnscheduledHandler;

  class EventPrincipal : public Principal {
  public:
    typedef EventAuxiliary Auxiliary;
    typedef std::vector<ProductProvenance> EntryInfoVector;

    typedef Principal Base;

    typedef Base::SharedConstGroupPtr SharedConstGroupPtr;
    EventPrincipal(EventAuxiliary const& aux,
                   cet::exempt_ptr<ProductRegistry const> reg,
        ProcessConfiguration const& pc,
        std::shared_ptr<History> history = std::shared_ptr<History>(new History),
        std::shared_ptr<BranchMapper> mapper = std::shared_ptr<BranchMapper>(new BranchMapper),
        std::shared_ptr<DelayedReader> rtrv = std::shared_ptr<DelayedReader>(new NoDelayedReader));

    // use compiler-generated copy c'tor, copy assignment, and d'tor

    SubRunPrincipal const& subRunPrincipal() const;

    SubRunPrincipal & subRunPrincipal();

    std::shared_ptr<SubRunPrincipal>
    subRunPrincipalSharedPtr() {
      return subRunPrincipal_;
    }

    void setSubRunPrincipal(std::shared_ptr<SubRunPrincipal> srp) {
      subRunPrincipal_ = srp;
    }

    EventID const& id() const {
      return aux().id();
    }

    Timestamp const& time() const {
      return aux().time();
    }

    bool isReal() const {
      return aux().isRealData();
    }

    EventAuxiliary::ExperimentType ExperimentType() const {
      return aux().experimentType();
    }

    EventAuxiliary const& aux() const {
      return aux_;
    }

    SubRunNumber_t subRun() const {
      return aux().subRun();
    }

    RunNumber_t run() const {
      return id().run();
    }

    RunPrincipal const& runPrincipal() const;

    RunPrincipal & runPrincipal();

    void addOnDemandGroup(ConstBranchDescription const& desc);

    void setUnscheduledHandler(std::shared_ptr<UnscheduledHandler> iHandler);

    EventSelectionIDVector const& eventSelectionIDs() const;

    History const& history() const {return *history_;}

    History& history() {return *history_;}

    Provenance
    getProvenance(ProductID const& pid) const;

    BasicHandle
    getByProductID(ProductID const& oid) const;

    void put(std::auto_ptr<EDProduct> edp, ConstBranchDescription const& bd,
         std::auto_ptr<ProductProvenance> productProvenance);

    void addGroup(ConstBranchDescription const& bd);

    void addGroup(std::auto_ptr<EDProduct> prod, ConstBranchDescription const& bd,
         std::auto_ptr<ProductProvenance> productProvenance);

    void addGroup(ConstBranchDescription const& bd, std::auto_ptr<ProductProvenance> productProvenance);

    void addGroup(std::auto_ptr<EDProduct> prod, ConstBranchDescription const& bd,
         std::shared_ptr<ProductProvenance> productProvenance);

    void addGroup(ConstBranchDescription const& bd, std::shared_ptr<ProductProvenance> productProvenance);

    virtual EDProduct const* getIt(ProductID const& pid) const;

    ProductID branchIDToProductID(BranchID const& bid) const;

    using Base::getProvenance;

    BranchType branchType() const { return InEvent; }

  private:

    BranchID productIDToBranchID(ProductID const& pid) const;

    virtual void addOrReplaceGroup(std::auto_ptr<Group> g);

    virtual ProcessHistoryID const& processHistoryID() const {return history().processHistoryID();}

    virtual void setProcessHistoryID(ProcessHistoryID const& phid) const {return history().setProcessHistoryID(phid);}

    virtual bool unscheduledFill(std::string const& moduleLabel) const;

    EventAuxiliary aux_;
    std::shared_ptr<SubRunPrincipal> subRunPrincipal_;

    // Handler for unscheduled modules
    std::shared_ptr<UnscheduledHandler> unscheduledHandler_;

    mutable std::vector<std::string> moduleLabelsRunning_;

    std::shared_ptr<History> history_;

    std::map<BranchListIndex, ProcessIndex> branchToProductIDHelper_;

  };

  inline
  bool
  isSameEvent(EventPrincipal const& a, EventPrincipal const& b) {
    return a.aux() == b.aux();
  }

}  // art

#endif /* art_Framework_Core_EventPrincipal_h */

// Local Variables:
// mode: c++
// End:
