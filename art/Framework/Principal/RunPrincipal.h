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
#include "art/Persistency/Provenance/BranchMapper.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/RunAuxiliary.h"
#include "cetlib/exempt_ptr.h"
#include "cpp0x/memory"
#include <vector>

namespace art {

class RunPrincipal : public Principal {

public:

  typedef RunAuxiliary Auxiliary;

public:

  virtual ~RunPrincipal();

  RunPrincipal(RunAuxiliary const&,
               ProcessConfiguration const&,
               std::unique_ptr<BranchMapper>&& mapper =
                 std::unique_ptr<BranchMapper>(new BranchMapper),
               std::unique_ptr<DelayedReader>&& rtrv =
                 std::unique_ptr<DelayedReader>(new NoDelayedReader),
               int idx = 0, RunPrincipal* = nullptr);

  RunAuxiliary const&
  aux() const
  {
    return aux_;
  }

  RunNumber_t
  run() const
  {
    return aux().run();
  }

  RunID const&
  id() const
  {
    return aux().id();
  }

  Timestamp const&
  beginTime() const
  {
    return aux().beginTime();
  }

  Timestamp const&
  endTime() const
  {
    return aux().endTime();
  }

  void
  setEndTime(Timestamp const& time)
  {
    aux_.setEndTime(time);
  }

  virtual BranchType branchType() const;

  void addGroup(BranchDescription const&);

  void addGroup(std::unique_ptr<EDProduct>&&, BranchDescription const&);

  void mergeRun(std::shared_ptr<RunPrincipal>);

  void put(std::unique_ptr<EDProduct>&&, BranchDescription const&,
           std::unique_ptr<ProductProvenance const>&&);

private:

  virtual void addOrReplaceGroup(std::unique_ptr<Group>&&);

  virtual ProcessHistoryID const& processHistoryID() const;

  virtual void setProcessHistoryID(ProcessHistoryID const&) const;

private:

  RunAuxiliary aux_;

};

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif // art_Framework_Principal_RunPrincipal_h
