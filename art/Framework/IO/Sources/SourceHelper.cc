#include "art/Framework/IO/Sources/SourceHelper.h"

#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"

#include <iomanip>
#include <iostream>
#include <utility>

using namespace std;

art::SourceHelper::SourceHelper(ModuleDescription const& md) :
  md_{md}
{}

art::RunPrincipal*
art::SourceHelper::makeRunPrincipal(RunAuxiliary const& runAux) const
{
  auto rp = new RunPrincipal{runAux, md_.processConfiguration()};
  // Add in groups for produced products so that we do not need deferred product getters anymore.
  //{
  //  auto const& pmd = ProductMetaData::instance();
  //  for (auto const& val : pmd.productList()) {
  //    auto const& bd = val.second;
  //    if ((bd.branchType() == InRun) && bd.produced()) {
  //      cout
  //          << "-----> SourceHelper::makeRunPrincipal: Creating group for produced product: "
  //          << "pid: "
  //          << bd.productID()
  //          << " branchName: "
  //          << bd.branchName()
  //          << endl;
  //      rp->Principal::fillGroup(rp, bd, RangeSet::invalid());
  //    }
  //  }
  //}
  return rp;
}

art::RunPrincipal*
art::SourceHelper::makeRunPrincipal(RunID const r, Timestamp const& startTime) const
{
  RunAuxiliary const runAux {r, startTime, Timestamp::invalidTimestamp()};
  auto rp = new RunPrincipal{runAux, md_.processConfiguration()};
  // Add in groups for produced products so that we do not need deferred product getters anymore.
  //{
  //  auto const& pmd = ProductMetaData::instance();
  //  for (auto const& val : pmd.productList()) {
  //    auto const& bd = val.second;
  //    if ((bd.branchType() == InRun) && bd.produced()) {
  //      cout
  //          << "-----> SourceHelper::makeRunPrincipal: Creating group for produced product: "
  //          << "pid: "
  //          << bd.productID()
  //          << " branchName: "
  //          << bd.branchName()
  //          << endl;
  //      rp->Principal::fillGroup(rp, bd, RangeSet::invalid());
  //    }
  //  }
  //}
  return rp;
}

art::RunPrincipal*
art::SourceHelper::makeRunPrincipal(RunNumber_t const r, Timestamp const& startTime) const
{
  auto rp = makeRunPrincipal(RunID{r}, startTime);
  return rp;
}

art::SubRunPrincipal*
art::SourceHelper::makeSubRunPrincipal(SubRunAuxiliary const& subRunAux) const
{
  auto srp = new SubRunPrincipal{subRunAux, md_.processConfiguration()};
  // Add in groups for produced products so that we do not need deferred product getters anymore.
  //{
  //  auto const& pmd = ProductMetaData::instance();
  //  for (auto const& val : pmd.productList()) {
  //    auto const& bd = val.second;
  //    if ((bd.branchType() == InSubRun) && bd.produced()) {
  //      cout
  //          << "-----> SourceHelper::makeSubRunPrincipal: Creating group for produced product: "
  //          << "pid: "
  //          << bd.productID()
  //          << " branchName: "
  //          << bd.branchName()
  //          << endl;
  //      srp->Principal::fillGroup(srp, bd, RangeSet::invalid());
  //    }
  //  }
  //}
  return srp;
}

art::SubRunPrincipal*
art::SourceHelper::makeSubRunPrincipal(SubRunID const& sr, Timestamp const& startTime) const
{
  SubRunAuxiliary const subRunAux {sr, startTime, Timestamp::invalidTimestamp()};
  auto srp = new SubRunPrincipal{subRunAux, md_.processConfiguration()};
  // Add in groups for produced products so that we do not need deferred product getters anymore.
  //{
  //  auto const& pmd = ProductMetaData::instance();
  //  for (auto const& val : pmd.productList()) {
  //    auto const& bd = val.second;
  //    if ((bd.branchType() == InSubRun) && bd.produced()) {
  //      cout
  //          << "-----> SourceHelper::makeSubRunPrincipal: Creating group for produced product: "
  //          << "pid: "
  //          << bd.productID()
  //          << " branchName: "
  //          << bd.branchName()
  //          << endl;
  //      srp->Principal::fillGroup(srp, bd, RangeSet::invalid());
  //    }
  //  }
  //}
  return srp;
}

art::SubRunPrincipal*
art::SourceHelper::makeSubRunPrincipal(RunNumber_t const r, SubRunNumber_t const sr, Timestamp const& startTime) const
{
  return makeSubRunPrincipal(SubRunID{r, sr}, startTime);
}

art::EventPrincipal*
art::SourceHelper::makeEventPrincipal(EventAuxiliary const& eventAux, std::unique_ptr<History>&& history) const
{
  auto ep = new EventPrincipal{eventAux, md_.processConfiguration(), std::move(history)};
  // Add in groups for produced products so that we do not need deferred product getters anymore.
  //{
  //  auto const& pmd = ProductMetaData::instance();
  //  for (auto const& val : pmd.productList()) {
  //    auto const& bd = val.second;
  //    if ((bd.branchType() == InEvent) && bd.produced()) {
  //      cout
  //          << "-----> SourceHelper::makeEventPrincipal: Creating group for produced product: "
  //          << "pid: "
  //          << bd.productID()
  //          << " branchName: "
  //          << bd.branchName()
  //          << endl;
  //      ep->Principal::fillGroup(ep, bd, RangeSet::invalid());
  //    }
  //  }
  //}
  return ep;
}

art::EventPrincipal*
art::SourceHelper::makeEventPrincipal(EventID const& e, Timestamp const& startTime, bool const isRealData,
                                      EventAuxiliary::ExperimentType const eType) const
{
  EventAuxiliary const eventAux {e, startTime, isRealData, eType};
  auto ep = new EventPrincipal{eventAux, md_.processConfiguration()};
  // Add in groups for produced products so that we do not need deferred product getters anymore.
  //{
  //  auto const& pmd = ProductMetaData::instance();
  //  for (auto const& val : pmd.productList()) {
  //    auto const& bd = val.second;
  //    if ((bd.branchType() == InEvent) && bd.produced()) {
  //      cout
  //          << "-----> SourceHelper::makeEventPrincipal: Creating group for produced product: "
  //          << "pid: "
  //          << bd.productID()
  //          << " branchName: "
  //          << bd.branchName()
  //          << endl;
  //      ep->Principal::fillGroup(ep, bd, RangeSet::invalid());
  //    }
  //  }
  //}
  return ep;
}

art::EventPrincipal*
art::SourceHelper::makeEventPrincipal(RunNumber_t const r, SubRunNumber_t const sr, EventNumber_t const e,
                                      Timestamp const& startTime, bool const isRealData,
                                      EventAuxiliary::ExperimentType const eType) const
{
  return makeEventPrincipal(EventID{r, sr, e}, startTime, isRealData, eType);
}

