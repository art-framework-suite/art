#include "art/Framework/IO/Sources/SourceHelper.h"

#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Provenance/ProductTables.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"

art::SourceHelper::SourceHelper(ModuleDescription const& md) :
  md_{md}
{}

void
art::SourceHelper::throwIfProductsNotRegistered_() const
{
  if (!presentProducts_) {
    throw Exception(errors::ProductRegistrationFailure,
                    "Error while attempting to create principal from SourceHelper.\n")
      << "Principals cannot be created until product registration is complete.\n"
      << "Perhaps you have attempted to create a Principal outside of your 'readNext'\n"
      << "function.  Please contact artists@fnal.gov for guidance.";
  }
}

void
art::SourceHelper::setPresentProducts(cet::exempt_ptr<ProductTables const> presentProducts)
{
  presentProducts_ = presentProducts;
}

art::RunPrincipal*
art::SourceHelper::makeRunPrincipal(RunAuxiliary const& runAux) const
{
  throwIfProductsNotRegistered_();
  return new RunPrincipal{runAux, md_.processConfiguration(), &presentProducts_->get(InRun)};
}

art::RunPrincipal*
art::SourceHelper::makeRunPrincipal(RunID const r,
                                    Timestamp const& startTime) const
{
  RunAuxiliary const runAux{r, startTime, Timestamp::invalidTimestamp()};
  return makeRunPrincipal(runAux);
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
  throwIfProductsNotRegistered_();
  return new SubRunPrincipal{subRunAux, md_.processConfiguration(), &presentProducts_->get(InSubRun)};
}

art::SubRunPrincipal*
art::SourceHelper::makeSubRunPrincipal(SubRunID const& sr,
                                       Timestamp const& startTime) const
{
  SubRunAuxiliary const subRunAux{sr, startTime, Timestamp::invalidTimestamp()};
  return makeSubRunPrincipal(subRunAux);
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
  throwIfProductsNotRegistered_();
  return new EventPrincipal{eventAux, md_.processConfiguration(), &presentProducts_->get(InEvent), history};
}

art::EventPrincipal*
art::SourceHelper::makeEventPrincipal(EventID const& e,
                                      Timestamp const& startTime,
                                      bool const isRealData,
                                      EventAuxiliary::ExperimentType const eType) const
{
  throwIfProductsNotRegistered_();
  EventAuxiliary const eventAux{e, startTime, isRealData, eType};
  return new EventPrincipal{eventAux, md_.processConfiguration(), &presentProducts_->get(InEvent)};
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
