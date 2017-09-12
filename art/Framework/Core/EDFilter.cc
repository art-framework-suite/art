#include "art/Framework/Core/EDFilter.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/ProducerBase.h"
#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/get_failureToPut_flag.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/CPCSentry.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib_except/demangle.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "hep_concurrency/SerialTaskQueueChain.h"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>

using namespace hep::concurrency;
using namespace std;

namespace art {

constexpr bool EDFilter::Pass;
constexpr bool EDFilter::Fail;

EDFilter::
~EDFilter()
{
}

one::
EDFilter::
~EDFilter()
{
}

stream::
EDFilter::
~EDFilter()
{
}

global::
EDFilter::
~EDFilter()
{
}

EDFilter::
EDFilter()
  : ProducerBase()
  , checkPutProducts_{true}
{
}

one::
EDFilter::
EDFilter()
  : art::EDFilter()
{
}

stream::
EDFilter::
EDFilter()
  : art::EDFilter()
{
}

global::
EDFilter::
EDFilter()
  : art::EDFilter()
{
}

string
EDFilter::
workerType() const
{
  return "WorkerT<EDFilter>";
}

void
EDFilter::
reconfigure(fhicl::ParameterSet const&)
{
}

void
EDFilter::
doBeginJob()
{
  uses(SharedResourcesRegistry::kLegacy);
  vector<string> names;
  for_each(resourceNames_.cbegin(), resourceNames_.cend(), [&names](string const& s){names.emplace_back(s);});
  auto queues = SharedResourcesRegistry::instance()->createQueues(names);
  chain_.reset(new SerialTaskQueueChain{queues});
  //cerr << "EDFilter::doBeginJob: chain_: " << hex << ((unsigned long*)chain_.get()) << dec << "\n";
  // Note: checkPutProducts_ cannot be set by the ctor
  // mbr init list since moduleDescription_ is empty there.
  auto const& mainID = moduleDescription().mainParameterSetID();
  auto const& scheduler_pset = fhicl::ParameterSetRegistry::get(mainID).get<fhicl::ParameterSet>("services.scheduler");
  auto const& module_pset = fhicl::ParameterSetRegistry::get(moduleDescription().parameterSetID());
  checkPutProducts_ = detail::get_failureToPut_flag(scheduler_pset, module_pset);
  //module_pset.get_if_present("errorOnMissingConsumes", requireConsumes_);
  //// Now that we know we have seen all the consumes declarations,
  //// sort the results for fast lookup later.
  //for (auto& vecPI : consumables_) {
  //  sort(vecPI.begin(), vecPI.end());
  //}
  beginJob();
}

void
one::
EDFilter::
doBeginJob()
{
  //uses(SharedResourcesRegistry::kLegacy);
  vector<string> names;
  for_each(resourceNames_.cbegin(), resourceNames_.cend(), [&names](string const& s){names.emplace_back(s);});
  auto queues = SharedResourcesRegistry::instance()->createQueues(names);
  chain_.reset(new SerialTaskQueueChain{queues});
  //cerr << "one::EDFilter::doBeginJob: chain_: " << hex << ((unsigned long*)chain_.get()) << dec << "\n";
  // Note: checkPutProducts_ cannot be set by the ctor
  // mbr init list since moduleDescription_ is empty there.
  auto const& mainID = moduleDescription().mainParameterSetID();
  auto const& scheduler_pset = fhicl::ParameterSetRegistry::get(mainID).get<fhicl::ParameterSet>("services.scheduler");
  auto const& module_pset = fhicl::ParameterSetRegistry::get(moduleDescription().parameterSetID());
  checkPutProducts_ = detail::get_failureToPut_flag(scheduler_pset, module_pset);
  //module_pset.get_if_present("errorOnMissingConsumes", requireConsumes_);
  //// Now that we know we have seen all the consumes declarations,
  //// sort the results for fast lookup later.
  //for (auto& vecPI : consumables_) {
  //  sort(vecPI.begin(), vecPI.end());
  //}
  beginJob();
}

void
stream::
EDFilter::
doBeginJob()
{
  //cerr << "stream::EDFilter::doBeginJob: chain_: " << hex << ((unsigned long*)chain_.get()) << dec << "\n";
  // Note: checkPutProducts_ cannot be set by the ctor
  // mbr init list since moduleDescription_ is empty there.
  auto const& mainID = moduleDescription().mainParameterSetID();
  auto const& scheduler_pset = fhicl::ParameterSetRegistry::get(mainID).get<fhicl::ParameterSet>("services.scheduler");
  auto const& module_pset = fhicl::ParameterSetRegistry::get(moduleDescription().parameterSetID());
  checkPutProducts_ = detail::get_failureToPut_flag(scheduler_pset, module_pset);
  //module_pset.get_if_present("errorOnMissingConsumes", requireConsumes_);
  //// Now that we know we have seen all the consumes declarations,
  //// sort the results for fast lookup later.
  //for (auto& vecPI : consumables_) {
  //  sort(vecPI.begin(), vecPI.end());
  //}
  beginJob();
}

void
global::
EDFilter::
doBeginJob()
{
  //cerr << "global::EDFilter::doBeginJob: chain_: " << hex << ((unsigned long*)chain_.get()) << dec << "\n";
  // Note: checkPutProducts_ cannot be set by the ctor
  // mbr init list since moduleDescription_ is empty there.
  auto const& mainID = moduleDescription().mainParameterSetID();
  auto const& scheduler_pset = fhicl::ParameterSetRegistry::get(mainID).get<fhicl::ParameterSet>("services.scheduler");
  auto const& module_pset = fhicl::ParameterSetRegistry::get(moduleDescription().parameterSetID());
  checkPutProducts_ = detail::get_failureToPut_flag(scheduler_pset, module_pset);
  //module_pset.get_if_present("errorOnMissingConsumes", requireConsumes_);
  //// Now that we know we have seen all the consumes declarations,
  //// sort the results for fast lookup later.
  //for (auto& vecPI : consumables_) {
  //  sort(vecPI.begin(), vecPI.end());
  //}
  beginJob();
}

void
EDFilter::
beginJob()
{
}

void
EDFilter::
doEndJob()
{
  endJob();
}

void
EDFilter::
endJob()
{
}

void
EDFilter::
doRespondToOpenInputFile(FileBlock const& fb)
{
  respondToOpenInputFile(fb);
}

void
EDFilter::
respondToOpenInputFile(FileBlock const&)
{
}

void
EDFilter::
doRespondToCloseInputFile(FileBlock const& fb)
{
  respondToCloseInputFile(fb);
}

void
EDFilter::
respondToCloseInputFile(FileBlock const&)
{
}

void
EDFilter::
doRespondToOpenOutputFiles(FileBlock const& fb)
{
  respondToOpenOutputFiles(fb);
}

void
EDFilter::
respondToOpenOutputFiles(FileBlock const&)
{
}

void
EDFilter::
doRespondToCloseOutputFiles(FileBlock const& fb)
{
  respondToCloseOutputFiles(fb);
}

void
EDFilter::
respondToCloseOutputFiles(FileBlock const&)
{
}

bool
EDFilter::
doBeginRun(RunPrincipal& rp, cet::exempt_ptr<CurrentProcessingContext const> cpc)
{
  detail::CPCSentry sentry{*cpc};
  Run r{rp, moduleDescription(), RangeSet::forRun(rp.runID())};
  bool const rc = beginRun(r);
  r.DataViewImpl::commit(rp);
  return rc;
}

bool
EDFilter::
beginRun(Run&)
{
  return true;
}

bool
EDFilter::
doEndRun(RunPrincipal& rp, cet::exempt_ptr<CurrentProcessingContext const> cpc)
{
  detail::CPCSentry sentry{*cpc};
  Run r{rp, moduleDescription(), rp.seenRanges()};
  bool const rc = endRun(r);
  r.DataViewImpl::commit(rp);
  return rc;
}

bool
EDFilter::
endRun(Run&)
{
  return true;
}

bool
EDFilter::
doBeginSubRun(SubRunPrincipal& srp, cet::exempt_ptr<CurrentProcessingContext const> cpc)
{
  detail::CPCSentry sentry{*cpc};
  SubRun sr{srp, moduleDescription(), RangeSet::forSubRun(srp.subRunID())};
  bool const rc = beginSubRun(sr);
  sr.DataViewImpl::commit(srp);
  return rc;
}

bool
EDFilter::
beginSubRun(SubRun&)
{
  return true;
}

bool
EDFilter::
doEndSubRun(SubRunPrincipal& srp, cet::exempt_ptr<CurrentProcessingContext const> cpc)
{
  detail::CPCSentry sentry{*cpc};
  SubRun sr{srp, moduleDescription(), srp.seenRanges()};
  bool const rc = endSubRun(sr);
  sr.DataViewImpl::commit(srp);
  return rc;
}

bool
EDFilter::
endSubRun(SubRun&)
{
  return true;
}

bool
EDFilter::
doEvent(EventPrincipal& ep, int /*si*/, CurrentProcessingContext const* cpc,
        atomic<size_t>& counts_run,
        atomic<size_t>& counts_passed,
        atomic<size_t>& counts_failed)
{
  detail::CPCSentry sentry{*cpc};
  Event e{ep, moduleDescription()};
  ++counts_run;
  bool rc = false;
  rc = filter(e);
  e.DataViewImpl::commit(ep, checkPutProducts_, &expectedProducts());
  if (rc) {
    ++counts_passed;
  }
  else {
    ++counts_failed;
  }
  return rc;
}

} // namespace art
