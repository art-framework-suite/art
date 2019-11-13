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
         "This can happen if you are attempting to create a Principal\n"
         "outside of your 'readNext' function, or if you are using a\n"
         "different SourceHelper object than the one provided by art.\n"
         "Please contact artists@fnal.gov for guidance.";
  }
}

art::ProcessHistoryID
art::SourceHelper::processHistoryID_(BranchType const bt,
                                     ProcessConfiguration const& pc) const
{
  art::ProcessHistory processHistory{};
  // If no products are present for this branch type, we do not
  // register the process history.
  if (presentProducts_->descriptions(bt).empty()) {
    return processHistory.id();
  }

  processHistory.push_back(pc);
  auto const phid = processHistory.id();
  art::ProcessHistoryRegistry::emplace(phid, processHistory);
  return phid;
}

std::unique_ptr<art::History>
art::SourceHelper::history_(ProcessConfiguration const& pc,
                            std::unique_ptr<History>&& history) const
{
  // If no event products are present, we do not register the process
  // history, and we return the already-provided history.
  if (presentProducts_->descriptions(InEvent).empty()) {
    return std::move(history);
  }

  // To append to the processing history, we must first retrieve the
  // ProcessHistory object corresponding to history's process history
  // ID, if the ID is valid.
  art::ProcessHistory processHistory;
  auto const phid = history->processHistoryID();
  if (phid.isValid()) {
    bool const success = ProcessHistoryRegistry::get(phid, processHistory);
    if (!success) {
      throw Exception{
        errors::LogicError,
        "Error while attempting to create event principal from SourceHelper.\n"}
        << "There is no processing history corresponding to the valid id: "
        << phid << '\n'
        << "Please contact artists@fnal.gov for guidance.";
    }
  }

  processHistory.push_back(pc);
  auto const new_phid = processHistory.id();
  art::ProcessHistoryRegistry::emplace(new_phid, processHistory);
  history->setProcessHistoryID(new_phid);
  return std::move(history);
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
    processHistoryID_(InRun, md_.processConfiguration());
  auto principal = new RunPrincipal{
    runAux, md_.processConfiguration(), &presentProducts_->get(InRun)};
  if (runAux.processHistoryID().isValid()) {
    principal->markProcessHistoryAsModified();
  }
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
  auto phid = processHistoryID_(InSubRun, md_.processConfiguration());
  subRunAux.setProcessHistoryID(phid);
  auto principal = new SubRunPrincipal{
    subRunAux, md_.processConfiguration(), &presentProducts_->get(InSubRun)};
  if (subRunAux.processHistoryID().isValid()) {
    principal->markProcessHistoryAsModified();
  }
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

art::EventPrincipal*
art::SourceHelper::makeEventPrincipal(EventAuxiliary const& eventAux,
                                      std::unique_ptr<History>&& history) const
{
  if (history.get() == nullptr) {
    throw Exception{
      errors::LogicError,
      "Error while attempting to create principal from SourceHelper.\n"}
      << "The provided 'unique_ptr<History>' object is null, which is not "
         "allowed\n"
         "for this makeEventPrincipal function overload.  Please choose a "
         "different\n"
         "function to use, or provided a non-null History unique pointer.  "
         "Contact\n"
         "artists@fnal.gov for further guidance.";
  }
  throwIfProductsNotRegistered_();
  auto new_history = history_(md_.processConfiguration(), std::move(history));
  auto const processHistoryID = new_history->processHistoryID();
  auto principal = new EventPrincipal{eventAux,
                                      md_.processConfiguration(),
                                      &presentProducts_->get(InEvent),
                                      std::move(new_history)};
  if (processHistoryID.isValid()) {
    principal->markProcessHistoryAsModified();
  }
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
