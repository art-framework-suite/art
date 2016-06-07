#include "art/Framework/IO/Sources/SourceHelper.h"

#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"

#include <cassert>
#include <memory>

art::SourceHelper::SourceHelper(ModuleDescription const & md) :
  md_(md)
{ }

art::RunPrincipal *
art::SourceHelper::makeRunPrincipal(RunAuxiliary const & runAux) const
{
  return new RunPrincipal{runAux, md_.processConfiguration()};
}

art::RunPrincipal *
art::SourceHelper::makeRunPrincipal(RunID r,
                                    Timestamp const & startTime) const
{
  RunAuxiliary runAux(r,
                      startTime,
                      Timestamp::invalidTimestamp());
  return new RunPrincipal{runAux, md_.processConfiguration()};
}

art::RunPrincipal *
art::SourceHelper::makeRunPrincipal(RunNumber_t r,
                                    Timestamp const & startTime) const
{
  return makeRunPrincipal(RunID{r}, startTime);
}

art::SubRunPrincipal *
art::SourceHelper::makeSubRunPrincipal(SubRunAuxiliary const & subRunAux
                                       ) const
{
  return new SubRunPrincipal{subRunAux, md_.processConfiguration()};
}

art::SubRunPrincipal *
art::SourceHelper::makeSubRunPrincipal(SubRunID const & sr,
                                       Timestamp const & startTime) const
{
  SubRunAuxiliary subRunAux(sr,
                            startTime,
                            Timestamp::invalidTimestamp());
  return new SubRunPrincipal{subRunAux, md_.processConfiguration()};
}

art::SubRunPrincipal *
art::SourceHelper::makeSubRunPrincipal(RunNumber_t r,
                                       SubRunNumber_t sr,
                                       Timestamp const & startTime) const
{
  return makeSubRunPrincipal(SubRunID{r, sr}, startTime);
}

art::EventPrincipal *
art::SourceHelper::makeEventPrincipal(EventAuxiliary const & eventAux,
                                      std::shared_ptr<History> && history
                                      ) const
{
  return new EventPrincipal{eventAux, md_.processConfiguration(), history};
}

art::EventPrincipal *
art::SourceHelper::makeEventPrincipal(EventID const & e,
                                      Timestamp const & startTime,
                                      bool isRealData,
                                      EventAuxiliary::ExperimentType eType) const
{
  EventAuxiliary eventAux(e, startTime, isRealData, eType);
  return new EventPrincipal{eventAux, md_.processConfiguration()};
}

art::EventPrincipal *
art::SourceHelper::makeEventPrincipal(RunNumber_t r,
                                      SubRunNumber_t sr,
                                      EventNumber_t e,
                                      Timestamp const & startTime,
                                      bool isRealData,
                                      EventAuxiliary::ExperimentType eType) const
{
  return makeEventPrincipal(EventID{r, sr, e}, startTime, isRealData, eType);
}
