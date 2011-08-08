#ifndef art_Framework_Principal_RunPrincipal_h
#define art_Framework_Principal_RunPrincipal_h

/*----------------------------------------------------------------------

RunPrincipal: This is the class responsible for management of
per run EDProducts. It is not seen by reconstruction code;
such code sees the Run class, which is a proxy for RunPrincipal.

The major internal component of the RunPrincipal
is the DataBlock.

----------------------------------------------------------------------*/

#include "art/Framework/Principal/NoDelayedReader.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/BranchMapper.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/RunAuxiliary.h"
#include "cetlib/exempt_ptr.h"
#include "cpp0x/memory"

#include <vector>

class art::RunPrincipal : public art::Principal {
public:
  typedef RunAuxiliary Auxiliary;

  RunPrincipal(RunAuxiliary const& aux,
               ProcessConfiguration const& pc,
               std::auto_ptr<BranchMapper> mapper = std::auto_ptr<BranchMapper>(new BranchMapper),
               std::auto_ptr<DelayedReader> rtrv = std::auto_ptr<DelayedReader>(new NoDelayedReader));
  ~RunPrincipal() {}

  RunAuxiliary const& aux() const { return aux_; }

  RunNumber_t run() const { return aux().run(); }

  RunID const& id() const { return aux().id(); }

  Timestamp const& beginTime() const { return aux().beginTime(); }

  Timestamp const& endTime() const { return aux().endTime(); }

  void setEndTime(Timestamp const& time) { aux_.setEndTime(time); }

  void mergeRun(std::shared_ptr<RunPrincipal> rp);

  void put(std::auto_ptr<EDProduct> edp,
           BranchDescription const& bd,
           std::auto_ptr<ProductProvenance const> productProvenance);

  void addGroup(BranchDescription const& bd);

  void addGroup(std::auto_ptr<EDProduct> prod,
                BranchDescription const& bd);

  BranchType branchType() const { return InRun; }

private:

  virtual void addOrReplaceGroup(std::auto_ptr<Group> g);

  virtual ProcessHistoryID const& processHistoryID() const {return aux().processHistoryID_;}

  virtual void setProcessHistoryID(ProcessHistoryID const& phid) const {return aux().setProcessHistoryID(phid);}

  RunAuxiliary aux_;
};

#endif /* art_Framework_Principal_RunPrincipal_h */

// Local Variables:
// mode: c++
// End:
