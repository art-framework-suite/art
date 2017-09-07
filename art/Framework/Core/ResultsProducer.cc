#include "art/Framework/Core/ResultsProducer.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/Core/RPWorkerT.h"
#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "art/Framework/Principal/Results.h"
#include "cetlib/PluginTypeDeducer.h"
#include "fhiclcpp/ParameterSet.h"
#include "hep_concurrency/SerialTaskQueueChain.h"

#include <memory>
#include <set>
#include <string>

using namespace hep::concurrency;
using namespace std;

string const
cet::PluginTypeDeducer<art::ResultsProducer>::value = "ResultsProducer";

namespace art {

ResultsProducer::
~ResultsProducer()
{
}

ResultsProducer::
ResultsProducer()
  : ModuleBase()
  , ProductRegistryHelper()
{
}

void
ResultsProducer::
registerProducts(MasterProductRegistry& mpr, ModuleDescription const& md)
{
  ProductRegistryHelper::registerProducts(mpr, md);
}

void
ResultsProducer::
doBeginJob()
{
  vector<string> names;
  for_each(resourceNames_.cbegin(), resourceNames_.cend(), [&names](string const& s){names.emplace_back(s);});
  auto queues = SharedResourcesRegistry::instance()->createQueues(names);
  //auto const& module_pset = fhicl::ParameterSetRegistry::get(moduleDescription().parameterSetID());
  //module_pset.get_if_present("errorOnMissingConsumes", requireConsumes_);
  //// Now that we know we have seen all the consumes declarations,
  /// sort the results for fast lookup later.
  //for (auto& vecPI : consumables_) {
  //  sort(vecPI.begin(), vecPI.end());
  //}
  beginJob();
}

void
ResultsProducer::
beginJob()
{
}

void
ResultsProducer::
doEndJob()
{
  endJob();
}

void
ResultsProducer::
endJob()
{
}

void
ResultsProducer::
doBeginRun(Run const& r)
{
  beginRun(r);
}

void
ResultsProducer::
beginRun(Run const&)
{
}

void
ResultsProducer::
doEndRun(Run const& r)
{
  endRun(r);
}

void
ResultsProducer::
endRun(Run const&)
{
}

void
ResultsProducer::
doBeginSubRun(SubRun const& sr)
{
  beginSubRun(sr);
}

void
ResultsProducer::
beginSubRun(SubRun const&)
{
}

void
ResultsProducer::
doEndSubRun(SubRun const& sr)
{
  endSubRun(sr);
}

void
ResultsProducer::
endSubRun(SubRun const&)
{
}

void
ResultsProducer::
doEvent(Event const& e)
{
  event(e);
}

void
ResultsProducer::
event(Event const&)
{
}

void
ResultsProducer::
doReadResults(Results const& res)
{
  readResults(res);
}

void
ResultsProducer::
readResults(Results const&)
{
}

void
ResultsProducer::
doWriteResults(ResultsPrincipal& /*rp*/, Results& res)
{
  writeResults(res);
  //res.commit_(rp);
  res.DataViewImpl::commit();
}

void
ResultsProducer::
doClear()
{
  clear();
}

} // namespace art

