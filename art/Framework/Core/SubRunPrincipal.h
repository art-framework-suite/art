#ifndef art_Framework_Core_SubRunPrincipal_h
#define art_Framework_Core_SubRunPrincipal_h

/*----------------------------------------------------------------------

SubRunPrincipal: This is the class responsible for management of
per subRun EDProducts. It is not seen by reconstruction code;
such code sees the SubRun class, which is a proxy for SubRunPrincipal.

The major internal component of the SubRunPrincipal
is the DataBlock.

----------------------------------------------------------------------*/

#include "art/Framework/Core/Principal.h"
#include "art/Persistency/Provenance/BranchMapper.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"
#include "cetlib/exempt_ptr.h"
#include "cpp0x/memory"
#include <vector>

namespace art {
  class RunPrincipal;
  class UnscheduledHandler;
  class SubRunPrincipal : public Principal {
  public:
    typedef SubRunAuxiliary Auxiliary;

    SubRunPrincipal(SubRunAuxiliary const& aux,
                    ProcessConfiguration const& pc,
                    std::auto_ptr<BranchMapper> mapper = std::auto_ptr<BranchMapper>(new BranchMapper),
                    std::shared_ptr<DelayedReader> rtrv = std::shared_ptr<DelayedReader>(new NoDelayedReader));

    ~SubRunPrincipal() {}

    RunPrincipal const& runPrincipal() const;

    RunPrincipal & runPrincipal();

    std::shared_ptr<RunPrincipal>
    runPrincipalSharedPtr() { return runPrincipal_; }

    void setRunPrincipal(std::shared_ptr<RunPrincipal> rp) { runPrincipal_ = rp; }

    SubRunID id() const { return aux().id(); }

    Timestamp const& beginTime() const { return aux().beginTime(); }

    Timestamp const& endTime() const { return aux().endTime(); }

    void setEndTime(Timestamp const& time) { aux_.setEndTime(time); }

    SubRunNumber_t subRun() const { return aux().subRun(); }

    SubRunAuxiliary const& aux() const { return aux_; }

    RunNumber_t run() const { return aux().run(); }

    void setUnscheduledHandler(std::shared_ptr<UnscheduledHandler>) {}

    void mergeSubRun(std::shared_ptr<SubRunPrincipal> srp);

    void put(std::auto_ptr<EDProduct> edp,
             BranchDescription const& bd,
             std::auto_ptr<ProductProvenance const> productProvenance);

    void addGroup(BranchDescription const& bd);

    void addGroup(std::auto_ptr<EDProduct> prod,
                  BranchDescription const& bd,
                  cet::exempt_ptr<ProductProvenance const> productProvenance);

    void addGroup(BranchDescription const& bd,
                  cet::exempt_ptr<ProductProvenance const> productProvenance);

    BranchType branchType() const { return InSubRun; }

  private:
    virtual void addOrReplaceGroup(std::auto_ptr<Group> g);

    virtual ProcessHistoryID const& processHistoryID() const {return aux().processHistoryID_;}

    virtual void setProcessHistoryID(ProcessHistoryID const& phid) const {return aux().setProcessHistoryID(phid);}

    virtual bool unscheduledFill(std::string const&) const {return false;}

    std::shared_ptr<RunPrincipal> runPrincipal_;
    SubRunAuxiliary aux_;
  };

}  // art

#endif /* art_Framework_Core_SubRunPrincipal_h */

// Local Variables:
// mode: c++
// End:
