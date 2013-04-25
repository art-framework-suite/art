#ifndef art_Framework_Core_PrincipalMaker_h
#define art_Framework_Core_PrincipalMaker_h

// -----------------------------------------------------------------
//
// PrincipalMaker provides the means for creation of EventPrincipals,
// SubRunPrincipals, and RunPrincipals.
//
// -----------------------------------------------------------------

#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/EventAuxiliary.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/History.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunID.h"

#include <memory>

namespace art {
  class ProcessConfiguration;
  class RunAuxiliary;
  class SubRunAuxiliary;
  class Timestamp;

  class PrincipalMaker {
  public:
    explicit PrincipalMaker(ProcessConfiguration const & pconf);

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
    ProcessConfiguration const & processConfig_;
  };
}


#endif /* art_Framework_Core_PrincipalMaker_h */

// Local Variables:
// mode: c++
// End:
