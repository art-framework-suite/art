#ifndef art_Framework_Principal_EventPrincipal_h
#define art_Framework_Principal_EventPrincipal_h
// vim: set sw=2:

//
//  EventPrincipal
//
//  Manages per-event data products.
//
//  This is not visible to modules, instead they use the Event class,
//  which is a proxy for this class.
//

#include "art/Framework/Principal/NoDelayedReader.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Provenance/BranchMapper.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/EventAuxiliary.h"
#include "art/Persistency/Provenance/History.h"
#include "cetlib/exempt_ptr.h"

#include <map>
#include <memory>
#include <vector>

namespace art {

class EventID;

class EventPrincipal : public Principal {

public:

  typedef EventAuxiliary Auxiliary;
  typedef Principal::SharedConstGroupPtr SharedConstGroupPtr;

public:

  virtual ~EventPrincipal();

  EventPrincipal(EventAuxiliary const& aux,
                 ProcessConfiguration const& pc,
                 std::shared_ptr<History> history =
                   std::shared_ptr<History>(new History),
                 std::unique_ptr<BranchMapper>&& mapper =
                   std::unique_ptr<BranchMapper>(new BranchMapper),
                 std::unique_ptr<DelayedReader>&& rtrv =
                   std::unique_ptr<DelayedReader>(new NoDelayedReader),
                 int idx = 0,
                 EventPrincipal* = nullptr);

  // use compiler-generated copy c'tor, copy assignment.

  SubRunPrincipal const& subRunPrincipal() const;

  SubRunPrincipal& subRunPrincipal();

  std::shared_ptr<SubRunPrincipal>
  subRunPrincipalSharedPtr()
  {
    return subRunPrincipal_;
  }

  void
  setSubRunPrincipal(std::shared_ptr<SubRunPrincipal> srp)
  {
    subRunPrincipal_ = srp;
  }

  EventID const&
  id() const
  {
    return aux().id();
  }

  Timestamp const&
  time() const
  {
    return aux().time();
  }

  bool
  isReal() const
  {
    return aux().isRealData();
  }

  EventAuxiliary::ExperimentType
  ExperimentType() const
  {
    return aux().experimentType();
  }

  EventAuxiliary const&
  aux() const
  {
    return aux_;
  }

  SubRunNumber_t
  subRun() const
  {
    return aux().subRun();
  }

  RunNumber_t
  run() const
  {
    return id().run();
  }

  RunPrincipal const& runPrincipal() const;

  RunPrincipal& runPrincipal();

  void addOnDemandGroup(BranchDescription const& desc,
                        cet::exempt_ptr<Worker> worker);

  EventSelectionIDVector const& eventSelectionIDs() const;

  History const&
  history() const
  {
    return *history_;
  }

  History&
  history()
  {
    return *history_;
  }

  using Principal::getGroup;

  GroupQueryResult getGroup(ProductID const& pid) const;

  GroupQueryResult getByProductID(ProductID const& pid) const;

  void put(std::unique_ptr<EDProduct>&& edp, BranchDescription const& bd,
           std::unique_ptr<ProductProvenance const>&& productProvenance);

  void addGroup(BranchDescription const&);

  void addGroup(std::unique_ptr<EDProduct>&&, BranchDescription const&);

  ProductID branchIDToProductID(BranchID const& bid) const;

  BranchType
  branchType() const
  {
    return InEvent;
  }

  EDProductGetter const* productGetter(ProductID const& pid) const;

private:

  BranchID productIDToBranchID(ProductID const& pid) const;

  virtual void addOrReplaceGroup(std::unique_ptr<Group>&& g);

  virtual
  ProcessHistoryID const&
  processHistoryID() const
  {
    return history().processHistoryID();
  }

  virtual
  void
  setProcessHistoryID(ProcessHistoryID const& phid) const
  {
    return history().setProcessHistoryID(phid);
  }

  // This function and its associated member datum are required to
  // handle the lifetime of a deferred getter, which in turn is required
  // because a group does not exist until it is placed in the event.
  EDProductGetter const* deferredGetter_(ProductID const& pid) const;

private:

  mutable
  std::map<ProductID, std::shared_ptr<DeferredProductGetter const>>
      deferredGetters_;

  EventAuxiliary aux_;

  std::shared_ptr<SubRunPrincipal> subRunPrincipal_;

  std::shared_ptr<History> history_;

  std::map<BranchListIndex, ProcessIndex> branchToProductIDHelper_;

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
#endif // art_Framework_Principal_EventPrincipal_h
