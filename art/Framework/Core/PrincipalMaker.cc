#include "art/Framework/Core/PrincipalMaker.h"

#include <cassert>

art::PrincipalMaker::PrincipalMaker(ProcessConfiguration const& pc) :
  processConfig_(pc)
  { }

art::RunPrincipal*
art::PrincipalMaker::makeRunPrincipal(RunNumber_t r,
                                      Timestamp const &startTime) const {

   RunAuxiliary runAux(RunID(r),
                       startTime,
                       Timestamp::invalidTimestamp());
   return new RunPrincipal(runAux, processConfig_);
}

art::SubRunPrincipal*
art::PrincipalMaker::makeSubRunPrincipal(RunNumber_t r,
                                         SubRunNumber_t sr,
                                         Timestamp const &startTime) const {
   SubRunAuxiliary subRunAux(SubRunID(r, sr),
                             startTime,
                             Timestamp::invalidTimestamp());
   return new SubRunPrincipal(subRunAux, processConfig_);
}

art::EventPrincipal*
art::PrincipalMaker::makeEventPrincipal(RunNumber_t r,
                                        SubRunNumber_t sr,
                                        EventNumber_t e,
                                        Timestamp const &startTime,
                                        bool isRealData,
                                        EventAuxiliary::ExperimentType eType) const {
   EventAuxiliary eventAux(EventID(r, sr, e),
                           startTime, isRealData, eType);
   return new EventPrincipal(eventAux, processConfig_);
}
