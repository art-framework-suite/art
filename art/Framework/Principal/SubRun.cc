#include "art/Framework/Principal/SubRun.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Provenance/BranchType.h"

namespace art {

  SubRun::~SubRun() {}

  SubRun::SubRun(SubRunPrincipal const& srp,
                 ModuleDescription const& md,
                 RangeSet const& rs /* = RangeSet::invalid() */)
    : DataViewImpl{InSubRun, srp, md, false, rs}
    , run_{srp.runPrincipalExemptPtr() ? new Run{srp.runPrincipal(), md} :
                                         nullptr}
  {}

  SubRunID
  SubRun::id() const
  {
    return DataViewImpl::subRunID();
  }

  Run const&
  SubRun::getRun() const
  {
    if (!run_) {
      throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL run.\n";
    }
    return *run_;
  }

} // namespace art
