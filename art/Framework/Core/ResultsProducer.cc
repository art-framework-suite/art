#include "art/Framework/Core/ResultsProducer.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Results.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Persistency/Provenance/ModuleContext.h"
#include "art/Utilities/SharedResource.h"

#include <string>

using namespace hep::concurrency;
using namespace std;

string const cet::PluginTypeDeducer<art::ResultsProducer>::value =
  "ResultsProducer";

namespace art {

  ResultsProducer::ResultsProducer() noexcept(false)
    : ProductRegistryHelper{product_creation_mode::produces}
  {
    serialize(detail::LegacyResource);
  }

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
    // If results producers ever become eligible for multi-threaded
    // execution, the serial task queues will need to be setup here.
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
    ModuleContext const mc{moduleDescription()};
    Run const r{rp, mc};
    beginRun(r);
  }

  void
  ResultsProducer::doEndRun(RunPrincipal const& rp)
  {
    ModuleContext const mc{moduleDescription()};
    Run const r{rp, mc};
    endRun(r);
  }

  void
  ResultsProducer::doBeginSubRun(SubRunPrincipal const& srp)
  {
    ModuleContext const mc{moduleDescription()};
    SubRun const sr{srp, mc};
    beginSubRun(sr);
  }

  void
  ResultsProducer::doEndSubRun(SubRunPrincipal const& srp)
  {
    ModuleContext const mc{moduleDescription()};
    SubRun const sr{srp, mc};
    endSubRun(sr);
  }

  void
  ResultsProducer::doEvent(EventPrincipal const& ep)
  {
    ModuleContext const mc{moduleDescription()};
    Event const e{ep, mc};
    event(e);
  }

  void
  ResultsProducer::doReadResults(ResultsPrincipal const& resp)
  {
    ModuleContext const mc{moduleDescription()};
    Results const res{resp, mc};
    readResults(res);
  }

  void
  ResultsProducer::doWriteResults(ResultsPrincipal& resp)
  {
    ModuleContext const mc{moduleDescription()};
    Results res{resp, mc};
    writeResults(res);
    res.movePutProductsToPrincipal(resp);
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
