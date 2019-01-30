#include "art/Framework/IO/Sources/SourceHelper.h"

#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "canvas/Persistency/Provenance/ProductTables.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"

#include <iomanip>
#include <iostream>
#include <utility>

using namespace std;

namespace {
  art::ProcessHistoryID
  insert_process_history(art::ProcessConfiguration const& pc)
  {
    art::ProcessHistory processHistory{};
    processHistory.push_back(pc);
    auto const phid = processHistory.id();
    art::ProcessHistoryRegistry::emplace(phid, processHistory);
    return phid;
  }
} // namespace

art::SourceHelper::SourceHelper(ModuleDescription const& md) : md_{md} {}

void
art::SourceHelper::throwIfProductsNotRegistered_() const
{
  if (!presentProducts_) {
    throw Exception(
      errors::ProductRegistrationFailure,
      "Error while attempting to create principal from SourceHelper.\n")
      << "Principals cannot be created until product registration is "
         "complete.\n"
      << "This can happen if you are attempting to create a Principal\n"
         "outside of your 'readNext' function, or if you are using a\n"
         "different SourceHelper object than the one provided by art.\n"
         "Please contact artists@fnal.gov for guidance.";
  }
}

void
art::SourceHelper::setPresentProducts(
  cet::exempt_ptr<ProductTables const> presentProducts)
{
  presentProducts_ = presentProducts;
}

art::RunPrincipal*
art::SourceHelper::makeRunPrincipal(RunAuxiliary const& runAux) const
{
  throwIfProductsNotRegistered_();
  runAux.processHistoryID() =
    insert_process_history(md_.processConfiguration());
  auto principal = new RunPrincipal{
    runAux, md_.processConfiguration(), &presentProducts_->get(InRun)};
  principal->markProcessHistoryAsModified();
  return principal;
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
  auto rp = makeRunPrincipal(RunID{r}, startTime);
  return rp;
}

art::SubRunPrincipal*
art::SourceHelper::makeSubRunPrincipal(SubRunAuxiliary const& subRunAux) const
{
  throwIfProductsNotRegistered_();
  auto phid = insert_process_history(md_.processConfiguration());
  subRunAux.setProcessHistoryID(phid);
  auto principal = new SubRunPrincipal{
    subRunAux, md_.processConfiguration(), &presentProducts_->get(InSubRun)};
  principal->markProcessHistoryAsModified();
  return principal;
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

// FIXME: What happens if history is nullptr?

art::EventPrincipal*
art::SourceHelper::makeEventPrincipal(EventAuxiliary const& eventAux,
                                      std::unique_ptr<History>&& history) const
{
  throwIfProductsNotRegistered_();
  auto const phid = insert_process_history(md_.processConfiguration());
  history->setProcessHistoryID(phid);
  auto principal = new EventPrincipal{eventAux,
                                      md_.processConfiguration(),
                                      &presentProducts_->get(InEvent),
                                      std::move(history)};
  principal->markProcessHistoryAsModified();
  return principal;
}

art::EventPrincipal*
art::SourceHelper::makeEventPrincipal(
  EventID const& e,
  Timestamp const& startTime,
  bool const isRealData,
  EventAuxiliary::ExperimentType const eType) const
{
  EventAuxiliary const eventAux{e, startTime, isRealData, eType};
  return makeEventPrincipal(eventAux, std::make_unique<History>());
}

art::EventPrincipal*
art::SourceHelper::makeEventPrincipal(
  RunNumber_t const r,
  SubRunNumber_t const sr,
  EventNumber_t const e,
  Timestamp const& startTime,
  bool const isRealData,
  EventAuxiliary::ExperimentType const eType) const
{
  return makeEventPrincipal(EventID{r, sr, e}, startTime, isRealData, eType);
}
