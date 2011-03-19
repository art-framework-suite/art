#ifndef art_Framework_Core_PrincipalMaker_h
#define art_Framework_Core_PrincipalMaker_h

// -----------------------------------------------------------------
//
// PrincipalMaker provides the means for creation of EventPrincipals,
// SubRunPrincipals, and RunPrincipals.
//
// -----------------------------------------------------------------

#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/Core/RunPrincipal.h"
#include "art/Framework/Core/SubRunPrincipal.h"

namespace art
{
  class PrincipalMaker
  {
  public:
    PrincipalMaker(ProductRegistry& preg,
                   ProcessConfiguration& pconf) :
      preg_(preg),
      processConfig_(pconf)
    { }

    RunPrincipal* makeRunPrincipal(RunNumber_t r,
                                   Timestamp const &startTime) const;

    SubRunPrincipal* makeSubRunPrincipal(RunNumber_t r,
                                         SubRunNumber_t sr,
                                         Timestamp const &startTime) const;

    EventPrincipal* makeEventPrincipal(RunNumber_t r,
                                       SubRunNumber_t sr,
                                       EventNumber_t e,
                                       Timestamp const &startTime,
                                       EventAuxiliary::ExperimentType eType =
                                       EventAuxiliary::Data) const;

  private:
    ProductRegistry&      preg_;
    ProcessConfiguration& processConfig_;

  };
}


#endif /* art_Framework_Core_PrincipalMaker_h */

// Local Variables:
// mode: c++
// End:
