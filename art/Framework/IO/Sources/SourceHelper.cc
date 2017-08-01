#include "art/Framework/IO/Sources/SourceHelper.h"

#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"

art::SourceHelper::SourceHelper(ModuleDescription const& md,
                                BranchTypeLookups const& productLookup,
                                BranchTypeLookups const& elementLookup) :
  md_{md},
  productLookup_{productLookup},
  elementLookup_{elementLookup}
{}

art::RunPrincipal*
art::SourceHelper::makeRunPrincipal(RunAuxiliary const& runAux) const
{
  return new RunPrincipal{runAux, md_.processConfiguration(), productLookup_, elementLookup_};
}

art::RunPrincipal*
art::SourceHelper::makeRunPrincipal(RunID const r,
                                    Timestamp const& startTime) const
{
  RunAuxiliary const runAux {r, startTime, Timestamp::invalidTimestamp()};
  return new RunPrincipal{runAux, md_.processConfiguration(), productLookup_, elementLookup_};
}

art::RunPrincipal*
art::SourceHelper::makeRunPrincipal(RunNumber_t const r,
                                    Timestamp const& startTime) const
{
  return makeRunPrincipal(RunID{r}, startTime);
}

art::SubRunPrincipal*
art::SourceHelper::makeSubRunPrincipal(SubRunAuxiliary const& subRunAux) const
{
  return new SubRunPrincipal{subRunAux, md_.processConfiguration(), productLookup_, elementLookup_};
}

art::SubRunPrincipal*
art::SourceHelper::makeSubRunPrincipal(SubRunID const& sr,
                                       Timestamp const& startTime) const
{
  SubRunAuxiliary const subRunAux {sr, startTime, Timestamp::invalidTimestamp()};
  return new SubRunPrincipal{subRunAux, md_.processConfiguration(), productLookup_, elementLookup_};
}

art::SubRunPrincipal*
art::SourceHelper::makeSubRunPrincipal(RunNumber_t const r,
                                       SubRunNumber_t const sr,
                                       Timestamp const& startTime) const
{
  return makeSubRunPrincipal(SubRunID{r, sr}, startTime);
}

art::EventPrincipal*
art::SourceHelper::makeEventPrincipal(EventAuxiliary const& eventAux,
                                      std::shared_ptr<History>&& history) const
{
  return new EventPrincipal{eventAux, md_.processConfiguration(), productLookup_, elementLookup_, history};
}

art::EventPrincipal*
art::SourceHelper::makeEventPrincipal(EventID const& e,
                                      Timestamp const& startTime,
                                      bool const isRealData,
                                      EventAuxiliary::ExperimentType const eType) const
{
  EventAuxiliary const eventAux {e, startTime, isRealData, eType};
  return new EventPrincipal{eventAux, md_.processConfiguration(), productLookup_, elementLookup_};
}

art::EventPrincipal*
art::SourceHelper::makeEventPrincipal(RunNumber_t const r,
                                      SubRunNumber_t const sr,
                                      EventNumber_t const e,
                                      Timestamp const& startTime,
                                      bool const isRealData,
                                      EventAuxiliary::ExperimentType const eType) const
{
  return makeEventPrincipal(EventID{r, sr, e}, startTime, isRealData, eType);
}
