#include "art/Framework/Core/PrincipalMaker.h"

#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Provenance/RunAuxiliary.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"

#include <cassert>

art::PrincipalMaker::PrincipalMaker(ProcessConfiguration const & pc) :
  processConfig_(pc)
{ }

art::RunPrincipal *
art::PrincipalMaker::makeRunPrincipal(RunID r,
                                      Timestamp const & startTime) const
{
  RunAuxiliary runAux(r,
                      startTime,
                      Timestamp::invalidTimestamp());
  return new RunPrincipal(runAux, processConfig_);
}

art::RunPrincipal *
art::PrincipalMaker::makeRunPrincipal(RunNumber_t r,
                                      Timestamp const & startTime) const
{
  return makeRunPrincipal(RunID(r), startTime);
}

art::SubRunPrincipal *
art::PrincipalMaker::makeSubRunPrincipal(SubRunID const & sr,
                                         Timestamp const & startTime) const
{
  SubRunAuxiliary subRunAux(sr,
                            startTime,
                            Timestamp::invalidTimestamp());
  return new SubRunPrincipal(subRunAux, processConfig_);
}

art::SubRunPrincipal *
art::PrincipalMaker::makeSubRunPrincipal(RunNumber_t r,
                                         SubRunNumber_t sr,
                                         Timestamp const & startTime) const
{
  return makeSubRunPrincipal(SubRunID(r, sr), startTime);
}

art::EventPrincipal *
art::PrincipalMaker::makeEventPrincipal(EventID const & e,
                                        Timestamp const & startTime,
                                        bool isRealData,
                                        EventAuxiliary::ExperimentType eType) const
{
  EventAuxiliary eventAux(e, startTime, isRealData, eType);
  return new EventPrincipal(eventAux, processConfig_);
}

art::EventPrincipal *
art::PrincipalMaker::makeEventPrincipal(RunNumber_t r,
                                        SubRunNumber_t sr,
                                        EventNumber_t e,
                                        Timestamp const & startTime,
                                        bool isRealData,
                                        EventAuxiliary::ExperimentType eType) const
{
  return makeEventPrincipal(EventID(r, sr, e), startTime, isRealData, eType);
}
