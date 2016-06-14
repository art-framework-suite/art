#ifndef art_Framework_Principal_SubRunPrincipal_h
#define art_Framework_Principal_SubRunPrincipal_h
// vim: set sw=2:

//
//  SubRunPrincipal
//
//  Manages per-subRun data products.
//
//  This is not visible to modules, instead they use the SubRun class,
//  which is a proxy for this class.
//

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

namespace art {

class SubRunPrincipal : public Principal {

public:

  typedef SubRunAuxiliary Auxiliary;

public:

  SubRunPrincipal(SubRunAuxiliary const&,
                  ProcessConfiguration const&,
                  std::unique_ptr<BranchMapper>&& mapper =
                    std::unique_ptr<BranchMapper>(new BranchMapper),
                  std::unique_ptr<DelayedReader>&& rtrv =
                    std::unique_ptr<DelayedReader>(new NoDelayedReader),
                  int idx = 0, SubRunPrincipal* = nullptr);


  RunPrincipal const& runPrincipal() const;

  RunPrincipal& runPrincipal();

  std::shared_ptr<RunPrincipal>
  runPrincipalSharedPtr()
  {
    return runPrincipal_;
  }

  void setRunPrincipal(std::shared_ptr<RunPrincipal> rp)
  {
    runPrincipal_ = rp;
  }

  SubRunID id() const
  {
    return aux().id();
  }

  Timestamp const& beginTime() const
  {
    return aux().beginTime();
  }

  Timestamp const& endTime() const
  {
    return aux().endTime();
  }

  void setEndTime(Timestamp const& time)
  {
    aux_.setEndTime(time);
  }

  SubRunNumber_t subRun() const
  {
    return aux().subRun();
  }

  SubRunAuxiliary const& aux() const
  {
    return aux_;
  }

  RunNumber_t run() const
  {
    return aux().run();
  }

  void mergeSubRun(std::shared_ptr<SubRunPrincipal>);

  void put(std::unique_ptr<EDProduct>&&, BranchDescription const&,
           std::unique_ptr<ProductProvenance const>&&);

  void addGroup(BranchDescription const&);

  void addGroup(std::unique_ptr<EDProduct>&&, BranchDescription const&);

  virtual BranchType branchType() const;

private:

  virtual void addOrReplaceGroup(std::unique_ptr<Group>&& g);

  virtual ProcessHistoryID const& processHistoryID() const;

  void setProcessHistoryID(ProcessHistoryID const& phid) override;

private:

  std::shared_ptr<RunPrincipal> runPrincipal_;
  SubRunAuxiliary aux_;

};

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_Principal_SubRunPrincipal_h */
