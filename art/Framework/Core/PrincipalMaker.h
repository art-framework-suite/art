#ifndef art_Framework_Core_PrincipalMaker_h
#define art_Framework_Core_PrincipalMaker_h

// -----------------------------------------------------------------
//
// PrincipalMaker provides the means for creation of EventPrincipals,
// SubRunPrincipals, and RunPrincipals.
//
// -----------------------------------------------------------------

#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"

namespace art
{
  class PrincipalMaker
  {
  public:
    explicit PrincipalMaker(ProcessConfiguration const& pconf);

    RunPrincipal* makeRunPrincipal(RunNumber_t r,
                                   Timestamp const &startTime) const;

    SubRunPrincipal* makeSubRunPrincipal(RunNumber_t r,
                                         SubRunNumber_t sr,
                                         Timestamp const &startTime) const;

    EventPrincipal* makeEventPrincipal(RunNumber_t r,
                                       SubRunNumber_t sr,
                                       EventNumber_t e,
                                       Timestamp const &startTime,
                                       bool isRealData = true,
                                       EventAuxiliary::ExperimentType eType =
                                       EventAuxiliary::Data) const;

  private:
    ProcessConfiguration const& processConfig_;
  };
}


#endif /* art_Framework_Core_PrincipalMaker_h */

// Local Variables:
// mode: c++
// End:
