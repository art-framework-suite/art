#include "art/Framework/Core/TriggerResultInserter.h"

#include "art/Framework/Principal/Event.h"
#include "art/Persistency/Common/TriggerResults.h"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"

using art::TriggerResultInserter;
using fhicl::ParameterSet;

TriggerResultInserter::TriggerResultInserter(const ParameterSet& pset, const TrigResPtr& trptr) :
  trptr_(trptr),
  pset_id_(pset.id())
{
  produces<TriggerResults>();
}

TriggerResultInserter::~TriggerResultInserter()
{ }

void TriggerResultInserter::produce(art::Event& e)
{
  std::unique_ptr<TriggerResults>
    results(new TriggerResults(*trptr_, pset_id_));

  e.put(results);
}
