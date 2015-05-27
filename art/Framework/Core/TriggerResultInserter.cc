#include "art/Framework/Core/TriggerResultInserter.h"

#include "art/Framework/Principal/Event.h"
#include "art/Persistency/Common/TriggerResults.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>

using art::TriggerResultInserter;
using fhicl::ParameterSet;

TriggerResultInserter::TriggerResultInserter(const ParameterSet& pset, HLTGlobalStatus & pathResults) :
  trptr_(&pathResults),
  pset_id_(pset.id())
{
  produces<TriggerResults>();
}

void TriggerResultInserter::produce(art::Event& e)
{
  e.put(std::unique_ptr<TriggerResults>(new TriggerResults(*trptr_, pset_id_)));
}
