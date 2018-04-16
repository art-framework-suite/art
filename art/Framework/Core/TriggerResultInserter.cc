#include "art/Framework/Core/TriggerResultInserter.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Event.h"
#include "canvas/Persistency/Common/TriggerResults.h"
#include "canvas/Utilities/DebugMacros.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>
#include <utility>

using namespace std;

namespace art {

  TriggerResultInserter::TriggerResultInserter(fhicl::ParameterSet const& pset,
                                               ScheduleID const si,
                                               HLTGlobalStatus& pathResults)
    : pset_id_{pset.id()}, trptr_(&pathResults)
  {
    {
      ostringstream msg;
      msg << "0x" << hex << ((unsigned long)this) << dec;
      TDEBUG_FUNC_SI_MSG(5, "TriggerResultInserter ctor", si, msg.str());
    }
    setScheduleID(si);
    if (si == ScheduleID::first()) {
      produces<TriggerResults>();
    }
  }

  void
  TriggerResultInserter::produce(Event& e)
  {
    auto tr = make_unique<TriggerResults>(*trptr_, pset_id_);
    e.put(move(tr));
  }

} // namespace art
