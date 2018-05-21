#include "art/Framework/Core/TriggerResultInserter.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Event.h"
#include "canvas/Persistency/Common/TriggerResults.h"
#include "canvas/Utilities/DebugMacros.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>
#include <sstream>
#include <utility>

namespace art {

  TriggerResultInserter::TriggerResultInserter(fhicl::ParameterSet const& pset,
                                               ScheduleID const sid,
                                               HLTGlobalStatus& pathResults)
    : ReplicatedProducer{pset, sid}, pset_id_{pset.id()}, trptr_(&pathResults)
  {
    {
      std::ostringstream msg;
      msg << "0x" << std::hex << ((unsigned long)this) << std::dec;
      TDEBUG_FUNC_SI_MSG(5, "TriggerResultInserter ctor", sid, msg.str());
    }
    produces<TriggerResults>();
  }

  void
  TriggerResultInserter::produce(Event& e)
  {
    auto tr = std::make_unique<TriggerResults>(*trptr_, pset_id_);
    e.put(move(tr));
  }

} // namespace art
