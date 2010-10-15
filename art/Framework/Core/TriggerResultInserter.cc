#include "art/Framework/Core/TriggerResultInserter.h"

#include "art/Framework/Core/Event.h"
#include "art/Persistency/Common/TriggerResults.h"

#include "fhiclcpp/ParameterSet.h"
  using fhicl::ParameterSet;

#include <memory>


namespace art {

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
    std::auto_ptr<TriggerResults>
      results(new TriggerResults(*trptr_, pset_id_));

    e.put(results);
  }

}  // namespace art
