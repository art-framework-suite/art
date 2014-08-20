#ifndef art_Framework_IO_Sources_SourceHelper_h
#define art_Framework_IO_Sources_SourceHelper_h

// -----------------------------------------------------------------
//
// SourceHelper provides the means for creation of EventPrincipals,
// SubRunPrincipals, and RunPrincipals.
//
// -----------------------------------------------------------------

#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/EventAuxiliary.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/History.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "art/Persistency/Provenance/TypeLabel.h"

#include <memory>

namespace art {
  class RunAuxiliary;
  class SubRunAuxiliary;
  class Timestamp;

  class SourceHelper;
}

class art::SourceHelper {
public:
  explicit SourceHelper(ModuleDescription const & md);

  template <typename T>
  Ptr<T>
  makePtr(TypeLabel const & t,
           EventPrincipal const & ep,
           typename Ptr<T>::key_type key) const;

  RunPrincipal * makeRunPrincipal(RunAuxiliary const & runAux) const;

  RunPrincipal * makeRunPrincipal(RunID r,
                                  Timestamp const & startTime) const;

  RunPrincipal * makeRunPrincipal(RunNumber_t r,
                                  Timestamp const & startTime) const;

  SubRunPrincipal * makeSubRunPrincipal(
    SubRunAuxiliary const & subRunAux) const;

  SubRunPrincipal * makeSubRunPrincipal(SubRunID const & sr,
                                        Timestamp const & startTime) const;

  SubRunPrincipal * makeSubRunPrincipal(RunNumber_t r,
                                        SubRunNumber_t sr,
                                        Timestamp const & startTime) const;

  EventPrincipal * makeEventPrincipal(EventAuxiliary const & eventAux,
                                      std::shared_ptr<History> && history
                                     ) const;

  EventPrincipal * makeEventPrincipal(EventID const & e,
                                      Timestamp const & startTime,
                                      bool isRealData = true,
                                      EventAuxiliary::ExperimentType eType =
                                      EventAuxiliary::Data) const;

  EventPrincipal * makeEventPrincipal(RunNumber_t r,
                                      SubRunNumber_t sr,
                                      EventNumber_t e,
                                      Timestamp const & startTime,
                                      bool isRealData = true,
                                      EventAuxiliary::ExperimentType eType =
                                      EventAuxiliary::Data) const;

private:
  ModuleDescription md_;
};

template <typename T>
art::Ptr<T>
art::SourceHelper::
makePtr(TypeLabel const & tl,
         EventPrincipal const & ep,
         typename Ptr<T>::key_type key) const
{
  auto pid =
    ep.branchIDToProductID(BranchDescription(tl, md_).branchID());
  return Ptr<T>(pid, key, ep.productGetter(pid));
}


#endif /* art_Framework_IO_Sources_SourceHelper_h */

// Local Variables:
// mode: c++
// End:
