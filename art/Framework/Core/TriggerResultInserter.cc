#include "art/Framework/Core/TriggerResultInserter.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Event.h"
#include "canvas/Persistency/Common/TriggerResults.h"
#include "canvas/Utilities/DebugMacros.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>

using namespace std;
using fhicl::ParameterSet;

namespace art {

  TriggerResultInserter::TriggerResultInserter(ParameterSet const& pset,
                                               int si,
                                               HLTGlobalStatus& pathResults)
    : pset_id_{pset.id()}, trptr_(&pathResults)
  {
    TDEBUG(5) << "TriggerResultInserter ctor: 0x" << hex
              << ((unsigned long)this) << dec << " (" << si << ")\n";
    setStreamIndex(si);
    if (si == 0) {
      produces<TriggerResults>();
    }
  }

  void
  TriggerResultInserter::produce(Event& e)
  {
    // No Event::get* calls should be made here!  The TriggerResults
    // object is self-contained, and it should have no parentage, which
    // an Event::get* call will introduce.
    e.put(make_unique<TriggerResults>(*trptr_, pset_id_));
  }

  // void
  // TriggerResultInserter::
  // produce_in_stream(Event& e, int /*si*/)
  //{
  //  // No Event::get* calls should be made here!  The TriggerResults
  //  // object is self-contained, and it should have no parentage, which
  //  // an Event::get* call will introduce.
  //  e.put(make_unique<TriggerResults>(*trptr_, pset_id_));
  //}

} // namespace art
