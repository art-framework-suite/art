#ifndef art_Framework_IO_Sources_SourceHelper_h
#define art_Framework_IO_Sources_SourceHelper_h

// -----------------------------------------------------------------
//
// SourceHelper provides the means for creation of EventPrincipals,
// SubRunPrincipals, and RunPrincipals.
//
// -----------------------------------------------------------------

#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Principal/Principal.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/History.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"

#include <memory>

namespace art {
  class RunAuxiliary;
  class SubRunAuxiliary;
  class Timestamp;

  class SourceHelper;
}

class art::SourceHelper {
public:
  explicit SourceHelper(ModuleDescription const& md);

  template <typename T>
  Ptr<T>
  makePtr(TypeLabel const& t,
          Principal const& p,
          typename Ptr<T>::key_type key) const;

  RunPrincipal* makeRunPrincipal(RunAuxiliary const& runAux) const;

  RunPrincipal* makeRunPrincipal(RunID r,
                                 Timestamp const& startTime) const;

  RunPrincipal* makeRunPrincipal(RunNumber_t r,
                                 Timestamp const& startTime) const;

  SubRunPrincipal* makeSubRunPrincipal(SubRunAuxiliary const& subRunAux) const;

  SubRunPrincipal* makeSubRunPrincipal(SubRunID const& sr,
                                       Timestamp const& startTime) const;

  SubRunPrincipal* makeSubRunPrincipal(RunNumber_t r,
                                       SubRunNumber_t sr,
                                       Timestamp const& startTime) const;

  EventPrincipal* makeEventPrincipal(EventAuxiliary const& eventAux,
                                     std::shared_ptr<History>&& history) const;

  EventPrincipal* makeEventPrincipal(EventID const& e,
                                     Timestamp const& startTime,
                                     bool isRealData = true,
                                     EventAuxiliary::ExperimentType eType = EventAuxiliary::Data) const;

  EventPrincipal* makeEventPrincipal(RunNumber_t r,
                                     SubRunNumber_t sr,
                                     EventNumber_t e,
                                     Timestamp const& startTime,
                                     bool isRealData = true,
                                     EventAuxiliary::ExperimentType eType = EventAuxiliary::Data) const;

private:
  ModuleDescription md_;
};

template <typename T>
art::Ptr<T>
art::SourceHelper::makePtr(TypeLabel const& tl,
                           Principal const& p,
                           typename Ptr<T>::key_type key) const
{
  BranchDescription const pd{p.branchType(), tl, md_};
  ProductID const pid{pd.productID()};
  return Ptr<T>{pid, key, p.productGetter(pid)};
}


#endif /* art_Framework_IO_Sources_SourceHelper_h */

// Local Variables:
// mode: c++
// End:
