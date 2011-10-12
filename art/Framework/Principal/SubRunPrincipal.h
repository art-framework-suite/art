#ifndef art_Framework_Principal_SubRunPrincipal_h
#define art_Framework_Principal_SubRunPrincipal_h

/*----------------------------------------------------------------------

SubRunPrincipal: This is the class responsible for management of
per subRun EDProducts. It is not seen by reconstruction code;
such code sees the SubRun class, which is a proxy for SubRunPrincipal.

The major internal component of the SubRunPrincipal
is the DataBlock.

----------------------------------------------------------------------*/

#include "art/Framework/Principal/NoDelayedReader.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/BranchMapper.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"
#include "cetlib/exempt_ptr.h"
#include "cpp0x/memory"

#include <vector>

class art::SubRunPrincipal : public art::Principal {
public:
  typedef SubRunAuxiliary Auxiliary;

  SubRunPrincipal(SubRunAuxiliary const & aux,
                  ProcessConfiguration const & pc,
                  std::auto_ptr<BranchMapper> mapper = std::auto_ptr<BranchMapper>(new BranchMapper),
                  std::auto_ptr<DelayedReader> rtrv = std::auto_ptr<DelayedReader>(new NoDelayedReader));

  ~SubRunPrincipal() {}

  RunPrincipal const & runPrincipal() const;

  RunPrincipal & runPrincipal();

  std::shared_ptr<RunPrincipal>
  runPrincipalSharedPtr() { return runPrincipal_; }

  void setRunPrincipal(std::shared_ptr<RunPrincipal> rp) { runPrincipal_ = rp; }

  SubRunID id() const { return aux().id(); }

  Timestamp const & beginTime() const { return aux().beginTime(); }

  Timestamp const & endTime() const { return aux().endTime(); }

  void setEndTime(Timestamp const & time) { aux_.setEndTime(time); }

  SubRunNumber_t subRun() const { return aux().subRun(); }

  SubRunAuxiliary const & aux() const { return aux_; }

  RunNumber_t run() const { return aux().run(); }

  void mergeSubRun(std::shared_ptr<SubRunPrincipal> srp);

  void put(std::auto_ptr<EDProduct> edp,
           BranchDescription const & bd,
           std::auto_ptr<ProductProvenance const> productProvenance);

  void addGroup(BranchDescription const & bd);

  void addGroup(std::auto_ptr<EDProduct> prod,
                BranchDescription const & bd);

  BranchType branchType() const { return InSubRun; }

private:
  virtual void addOrReplaceGroup(std::auto_ptr<Group> g);

  virtual ProcessHistoryID const & processHistoryID() const {return aux().processHistoryID_;}

  virtual void setProcessHistoryID(ProcessHistoryID const & phid) const {return aux().setProcessHistoryID(phid);}

  std::shared_ptr<RunPrincipal> runPrincipal_;
  SubRunAuxiliary aux_;
};

#endif /* art_Framework_Principal_SubRunPrincipal_h */

// Local Variables:
// mode: c++
// End:
