#include "art/Framework/Core/ResultsProducer.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/Core/RPWorkerT.h"
#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Results.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "cetlib/PluginTypeDeducer.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>
#include <set>
#include <string>

using namespace hep::concurrency;
using namespace std;

string const cet::PluginTypeDeducer<art::ResultsProducer>::value =
  "ResultsProducer";

namespace art {

  void
  ResultsProducer::registerProducts(ProductDescriptions& productsToProduce,
                                    ModuleDescription const& md)
  {
    ProductRegistryHelper::registerProducts(productsToProduce, md);
    setModuleDescription(md);
  }

  void
  ResultsProducer::doBeginJob()
  {
    vector<string> const names(cbegin(resourceNames_), cend(resourceNames_));
    auto queues = SharedResourcesRegistry::instance()->createQueues(names);
    beginJob();
  }

  void
  ResultsProducer::doEndJob()
  {
    endJob();
  }

  void
  ResultsProducer::doBeginRun(RunPrincipal const& rp)
  {
    Run const r{rp, md_};
    beginRun(r);
  }

  void
  ResultsProducer::doEndRun(RunPrincipal const& rp)
  {
    Run const r{rp, md_};
    endRun(r);
  }

  void
  ResultsProducer::doBeginSubRun(SubRunPrincipal const& srp)
  {
    SubRun const sr{srp, md_};
    beginSubRun(sr);
  }

  void
  ResultsProducer::doEndSubRun(SubRunPrincipal const& srp)
  {
    SubRun const sr{srp, md_};
    endSubRun(sr);
  }

  void
  ResultsProducer::doEvent(EventPrincipal const& ep)
  {
    Event const e{ep, md_};
    event(e);
  }

  void
  ResultsProducer::doReadResults(ResultsPrincipal const& resp)
  {
    Results const res{resp, md_};
    readResults(res);
  }

  void
  ResultsProducer::doWriteResults(ResultsPrincipal& resp)
  {
    Results res{resp, md_};
    writeResults(res);
    res.DataViewImpl::movePutProductsToPrincipal(resp);
  }

  void
  ResultsProducer::doClear()
  {
    clear();
  }

  // Virtual functions to be overridden by users
  void
  ResultsProducer::readResults(Results const&)
  {}

  void
  ResultsProducer::beginJob()
  {}

  void
  ResultsProducer::endJob()
  {}

  void
  ResultsProducer::beginRun(Run const&)
  {}

  void
  ResultsProducer::endRun(Run const&)
  {}

  void
  ResultsProducer::beginSubRun(SubRun const&)
  {}

  void
  ResultsProducer::endSubRun(SubRun const&)
  {}

  void
  ResultsProducer::event(Event const&)
  {}

} // namespace art
