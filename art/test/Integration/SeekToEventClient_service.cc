#include "art/Framework/IO/Root/RootInput.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/test/Integration/SeekToEventClient.h"
#include "fhiclcpp/ParameterSet.h"

#include <cassert>
#include <iostream>

arttest::SeekToEventClient::SeekToEventClient(Parameters const& config,
                                              art::ActivityRegistry& r)
  : nextEventsToProcess_{config().nextEventsToProcess()}
{
  r.sPostBeginJobWorkers.watch(this, &SeekToEventClient::postBeginJobWorkers);
  r.sPostProcessEvent.watch(this, &SeekToEventClient::preProcessEvent);
  r.sPostProcessEvent.watch(this, &SeekToEventClient::postProcessEvent);
}

void
arttest::SeekToEventClient::postBeginJobWorkers(art::InputSource* input_source,
                                                std::vector<art::Worker*> const&)
{
  if (nextEventsToProcess_.empty()) return;

  input_ = dynamic_cast<art::RootInput*>(input_source);
  if (!input_)
    throw art::Exception(art::errors::Configuration)
      << "The SeekToEventClient service can be used only with\n"
      << "the RootInput source.\n";
}

void
arttest::SeekToEventClient::preProcessEvent(art::Event const& e)
{
  // Check to make sure SubRuns and Runs are appropriately filled.
  // Exception will be thrown if not.  Discard return value.
  e.getSubRun();
  e.getRun();
}

void
arttest::SeekToEventClient::postProcessEvent(art::Event const&)
{
  if (nextEventsToProcess_.empty()) return;
  input_->seekToEvent(nextEventsToProcess_[0]);
  nextEventsToProcess_.erase(nextEventsToProcess_.begin());
}

DEFINE_ART_SERVICE(arttest::SeekToEventClient)
