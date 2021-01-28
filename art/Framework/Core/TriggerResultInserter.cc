#include "art/Framework/Core/TriggerResultInserter.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Event.h"
#include "art/Utilities/TaskDebugMacros.h"
#include "canvas/Persistency/Common/TriggerResults.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>
#include <sstream>
#include <utility>

namespace art {

  TriggerResultInserter::TriggerResultInserter(fhicl::ParameterSet const& pset,
                                               ScheduleID const sid,
                                               HLTGlobalStatus& pathResults)
    : ReplicatedProducer{pset, ProcessingFrame{sid}}
    , pset_id_{pset.id()}
    , trptr_{&pathResults}
  {
    TDEBUG_FUNC_SI(5, sid) << std::hex << this << std::dec;
    produces<TriggerResults>();
  }

  void
  TriggerResultInserter::produce(Event& e, ProcessingFrame const&)
  {
    auto tr = std::make_unique<TriggerResults>(*trptr_, pset_id_);
    e.put(move(tr));
  }

} // namespace art
