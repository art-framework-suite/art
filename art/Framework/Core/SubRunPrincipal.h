#ifndef FWCore_Framework_SubRunPrincipal_h
#define FWCore_Framework_SubRunPrincipal_h

/*----------------------------------------------------------------------

SubRunPrincipal: This is the class responsible for management of
per subRun EDProducts. It is not seen by reconstruction code;
such code sees the SubRun class, which is a proxy for SubRunPrincipal.

The major internal component of the SubRunPrincipal
is the DataBlock.



----------------------------------------------------------------------*/

#include "art/Framework/Core/Principal.h"
#include "art/Persistency/Provenance/BranchMapper.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"
#include "boost/shared_ptr.hpp"
#include <vector>

namespace art {
  class RunPrincipal;
  class UnscheduledHandler;
  class SubRunPrincipal : public Principal {
  public:
    typedef SubRunAuxiliary Auxiliary;
    typedef std::vector<ProductProvenance> EntryInfoVector;
    typedef Principal Base;
    SubRunPrincipal(SubRunAuxiliary const& aux,
        boost::shared_ptr<ProductRegistry const> reg,
        ProcessConfiguration const& pc,
        boost::shared_ptr<BranchMapper> mapper = boost::shared_ptr<BranchMapper>(new BranchMapper),
        boost::shared_ptr<DelayedReader> rtrv = boost::shared_ptr<DelayedReader>(new NoDelayedReader));

    ~SubRunPrincipal() {}

    RunPrincipal const& runPrincipal() const {
      return *runPrincipal_;
    }

    RunPrincipal & runPrincipal() {
      return *runPrincipal_;
    }

    boost::shared_ptr<RunPrincipal>
    runPrincipalSharedPtr() {
      return runPrincipal_;
    }

    void setRunPrincipal(boost::shared_ptr<RunPrincipal> rp) {
      runPrincipal_ = rp;
    }

    SubRunID id() const {
      return aux().id();
    }

    Timestamp const& beginTime() const {
      return aux().beginTime();
    }

    Timestamp const& endTime() const {
      return aux().endTime();
    }

    void setEndTime(Timestamp const& time) {
      aux_.setEndTime(time);
    }

    SubRunNumber_t subRun() const {
      return aux().subRun();
    }

    SubRunAuxiliary const& aux() const {
      return aux_;
    }

    RunNumber_t run() const {
      return aux().run();
    }

    void setUnscheduledHandler(boost::shared_ptr<UnscheduledHandler>) {}

    void mergeSubRun(boost::shared_ptr<SubRunPrincipal> srp);

    void put(std::auto_ptr<EDProduct> edp,
             ConstBranchDescription const& bd, std::auto_ptr<ProductProvenance> productProvenance);

    void addGroup(ConstBranchDescription const& bd);

    void addGroup(std::auto_ptr<EDProduct> prod, ConstBranchDescription const& bd, std::auto_ptr<ProductProvenance> productProvenance);

    void addGroup(ConstBranchDescription const& bd, std::auto_ptr<ProductProvenance> productProvenance);

  private:
    virtual void addOrReplaceGroup(std::auto_ptr<Group> g);

    virtual ProcessHistoryID const& processHistoryID() const {return aux().processHistoryID_;}

    virtual void setProcessHistoryID(ProcessHistoryID const& phid) const {return aux().setProcessHistoryID(phid);}

    virtual bool unscheduledFill(std::string const&) const {return false;}

    boost::shared_ptr<RunPrincipal> runPrincipal_;
    SubRunAuxiliary aux_;
  };

}  // art

#endif
