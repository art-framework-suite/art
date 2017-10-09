#include "art/Framework/Core/TriggerResultInserter.h"

#include "art/Framework/Principal/Event.h"
#include "canvas/Persistency/Common/TriggerResults.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>

using art::TriggerResultInserter;
using fhicl::ParameterSet;

TriggerResultInserter::TriggerResultInserter(ParameterSet const& pset,
                                             HLTGlobalStatus& pathResults)
  : trptr_{&pathResults}, pset_id_{pset.id()}
{
  produces<TriggerResults>();
}

void
TriggerResultInserter::produce(art::Event& e)
{
  // No Event::get* calls should be made here!  The TriggerResults
  // object is self-contained, and it should have no parentage, which
  // an Event::get* call will introduce.
  e.put(std::make_unique<TriggerResults>(*trptr_, pset_id_));
}
