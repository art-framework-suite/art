#include "art/Framework/IO/Sources/SourceHelper.h"

#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "canvas/Persistency/Provenance/ProductTables.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"

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

void
art::SourceHelper::setPresentProducts(
  cet::exempt_ptr<ProductTables const> presentProducts)
{
  presentProducts_ = presentProducts;
}

template <typename T>
T*
art::SourceHelper::makePrincipal_(typename T::Auxiliary aux) const
{
  throwIfProductsNotRegistered_();
  constexpr auto branch_type = T::branch_type;
  aux.setProcessHistoryID(
    processHistoryID_(branch_type, md_.processConfiguration()));
  auto principal =
    new T{aux, md_.processConfiguration(), &presentProducts_->get(branch_type)};
  if (aux.processHistoryID().isValid()) {
    principal->markProcessHistoryAsModified();
  }
  return principal;
}

art::RunPrincipal*
art::SourceHelper::makeRunPrincipal(RunAuxiliary runAux) const
{
  return makePrincipal_<RunPrincipal>(std::move(runAux));
}

art::RunPrincipal*
art::SourceHelper::makeRunPrincipal(RunID const r,
                                    Timestamp const& startTime) const
{
  return makeRunPrincipal(
    RunAuxiliary{r, startTime, Timestamp::invalidTimestamp()});
}

art::RunPrincipal*
art::SourceHelper::makeRunPrincipal(RunNumber_t const r,
                                    Timestamp const& startTime) const
{
  return makeRunPrincipal(RunID{r}, startTime);
}

art::SubRunPrincipal*
art::SourceHelper::makeSubRunPrincipal(SubRunAuxiliary subRunAux) const
{
  return makePrincipal_<SubRunPrincipal>(std::move(subRunAux));
}

art::SubRunPrincipal*
art::SourceHelper::makeSubRunPrincipal(SubRunID const& sr,
                                       Timestamp const& startTime) const
{
  return makeSubRunPrincipal(
    SubRunAuxiliary{sr, startTime, Timestamp::invalidTimestamp()});
}

art::SubRunPrincipal*
art::SourceHelper::makeSubRunPrincipal(RunNumber_t const r,
                                       SubRunNumber_t const sr,
                                       Timestamp const& startTime) const
{
  return makeSubRunPrincipal(SubRunID{r, sr}, startTime);
}

art::EventPrincipal*
art::SourceHelper::makeEventPrincipal(EventAuxiliary eventAux) const
{
  return makePrincipal_<EventPrincipal>(std::move(eventAux));
}

art::EventPrincipal*
art::SourceHelper::makeEventPrincipal(
  EventID const& e,
  Timestamp const& startTime,
  bool const isRealData,
  EventAuxiliary::ExperimentType const eType) const
{
  return makeEventPrincipal(EventAuxiliary{e, startTime, isRealData, eType});
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
