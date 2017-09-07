#include "art/Framework/Core/EDAnalyzer.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/EventObserverBase.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/IgnoreModuleLabel.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/CPCSentry.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib_except/demangle.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/ConfigurationTable.h"
#include "fhiclcpp/types/KeysToIgnore.h"
#include "fhiclcpp/types/OptionalTable.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/TableFragment.h"
#include "hep_concurrency/SerialTaskQueueChain.h"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <memory>
#include <ostream>
#include <string>

using namespace hep::concurrency;
using namespace std;

namespace art {

EDAnalyzer::
~EDAnalyzer()
{
}

one::
EDAnalyzer::
~EDAnalyzer()
{
}

stream::
EDAnalyzer::
~EDAnalyzer()
{
}

global::
EDAnalyzer::
~EDAnalyzer()
{
}

EDAnalyzer::
EDAnalyzer(fhicl::ParameterSet const& pset)
  : EventObserverBase{pset}
{
}

one::
EDAnalyzer::
EDAnalyzer(fhicl::ParameterSet const& pset)
  : art::EDAnalyzer{pset}
{
}

stream::
EDAnalyzer::
EDAnalyzer(fhicl::ParameterSet const& pset)
  : art::EDAnalyzer{pset}
{
}

global::
EDAnalyzer::
EDAnalyzer(fhicl::ParameterSet const& pset)
  : art::EDAnalyzer{pset}
{
}

string
EDAnalyzer::
workerType() const
{
  return "WorkerT<EDAnalyzer>";
}

// Not called by framework
void
EDAnalyzer::
reconfigure(fhicl::ParameterSet const&)
{
}

void
EDAnalyzer::
doBeginJob()
{
  uses(SharedResourcesRegistry::kLegacy);
  vector<string> names;
  for_each(resourceNames_.cbegin(), resourceNames_.cend(), [&names](string const& s){names.emplace_back(s);});
  auto queues = SharedResourcesRegistry::instance()->createQueues(names);
  chain_.reset(new SerialTaskQueueChain{queues});
  //cerr << "EDAnalyzer::doBeginJob: chain_: " << hex << ((unsigned long*)chain_.get()) << dec << "\n";
  //// Now that we know we have seen all the consumes declarations,
  //// sort the results for fast lookup later.
  //for (auto& vecPI : consumables_) {
  //  sort(vecPI.begin(), vecPI.end());
  //}
  beginJob();
}

void
one::
EDAnalyzer::
doBeginJob()
{
  //uses(SharedResourcesRegistry::kLegacy);
  vector<string> names;
  for_each(resourceNames_.cbegin(), resourceNames_.cend(), [&names](string const& s){names.emplace_back(s);});
  auto queues = SharedResourcesRegistry::instance()->createQueues(names);
  chain_.reset(new SerialTaskQueueChain{queues});
  //cerr << "one::EDAnalyzer::doBeginJob: chain_: " << hex << ((unsigned long*)chain_.get()) << dec << "\n";
  //// Now that we know we have seen all the consumes declarations,
  //// sort the results for fast lookup later.
  //for (auto& vecPI : consumables_) {
  //  sort(vecPI.begin(), vecPI.end());
  //}
  beginJob();
}

void
stream::
EDAnalyzer::
doBeginJob()
{
  //cerr << "stream::EDAnalyzer::doBeginJob: chain_: " << hex << ((unsigned long*)chain_.get()) << dec << "\n";
  //// Now that we know we have seen all the consumes declarations,
  //// sort the results for fast lookup later.
  //for (auto& vecPI : consumables_) {
  //  sort(vecPI.begin(), vecPI.end());
  //}
  beginJob();
}

void
global::
EDAnalyzer::
doBeginJob()
{
  //cerr << "global::EDAnalyzer::doBeginJob: chain_: " << hex << ((unsigned long*)chain_.get()) << dec << "\n";
  //// Now that we know we have seen all the consumes declarations,
  //// sort the results for fast lookup later.
  //for (auto& vecPI : consumables_) {
  //  sort(vecPI.begin(), vecPI.end());
  //}
  beginJob();
}

void
EDAnalyzer::
beginJob()
{
}

void
EDAnalyzer::
doEndJob()
{
  endJob();
}

void
EDAnalyzer::
endJob()
{
}

void
EDAnalyzer::
doRespondToOpenInputFile(FileBlock const& fb)
{
  respondToOpenInputFile(fb);
}

void
EDAnalyzer::
respondToOpenInputFile(FileBlock const&)
{
}

void
EDAnalyzer::
doRespondToCloseInputFile(FileBlock const& fb)
{
  respondToCloseInputFile(fb);
}

void
EDAnalyzer::
respondToCloseInputFile(FileBlock const&)
{
}

void
EDAnalyzer::
doRespondToOpenOutputFiles(FileBlock const& fb)
{
  respondToOpenOutputFiles(fb);
}

void
EDAnalyzer::
respondToOpenOutputFiles(FileBlock const&)
{
}

void
EDAnalyzer::
doRespondToCloseOutputFiles(FileBlock const& fb)
{
  respondToCloseOutputFiles(fb);
}

void
EDAnalyzer::
respondToCloseOutputFiles(FileBlock const&)
{
}

bool
EDAnalyzer::
doBeginRun(RunPrincipal& rp, cet::exempt_ptr<CurrentProcessingContext const> cpc)
{
  detail::CPCSentry sentry{*cpc};
  Run const r{rp, moduleDescription()};
  beginRun(r);
  return true;
}

void
EDAnalyzer::
beginRun(Run const&)
{
}

bool
EDAnalyzer::
doEndRun(RunPrincipal& rp, cet::exempt_ptr<CurrentProcessingContext const> cpc)
{
  detail::CPCSentry sentry{*cpc};
  Run const r{rp, moduleDescription()};
  endRun(r);
  return true;
}

void
EDAnalyzer::
endRun(Run const&)
{
}

bool
EDAnalyzer::
doBeginSubRun(SubRunPrincipal& srp, cet::exempt_ptr<CurrentProcessingContext const> cpc)
{
  detail::CPCSentry sentry{*cpc};
  SubRun const sr{srp, moduleDescription()};
  beginSubRun(sr);
  return true;
}

void
EDAnalyzer::
beginSubRun(SubRun const&)
{
}

bool
EDAnalyzer::
doEndSubRun(SubRunPrincipal& srp, cet::exempt_ptr<CurrentProcessingContext const> cpc)
{
  detail::CPCSentry sentry{*cpc};
  SubRun const sr{srp, moduleDescription()};
  endSubRun(sr);
  return true;
}

void
EDAnalyzer::
endSubRun(SubRun const&)
{
}

bool
EDAnalyzer::
doEvent(EventPrincipal& ep, int /*si*/, CurrentProcessingContext const* cpc,
        std::atomic<std::size_t>& counts_run,
        std::atomic<std::size_t>& counts_passed,
        std::atomic<std::size_t>& /*counts_failed*/)
{
  detail::CPCSentry sentry{*cpc};
  detail::PVSentry pvSentry {cachedProducts()};
  Event const e{ep, moduleDescription()};
  if (wantAllEvents() || wantEvent(e)) {
    ++counts_run;
    //if (static_cast<ModuleThreadingType>(moduleDescription().moduleThreadingType()) == ModuleThreadingType::STREAM) {
      //analyze_in_stream(e, si);
    //}
    //else {
      analyze(e);
    //}
    ++counts_passed;
  }
  return true;
}

//void
//EDAnalyzer::
//analyze_in_stream(Event const&, int /*si*/)
//{
//}

} // namespace art

