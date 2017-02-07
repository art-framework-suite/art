#ifndef art_test_Framework_Principal_SimpleEvent_h
#define art_test_Framework_Principal_SimpleEvent_h

#include "canvas/Persistency/Provenance/EventID.h"

namespace arttest {

  struct SimpleEvent {
    SimpleEvent(art::EventID const& eid, bool const last)
      : id{eid}
      , lastInSubRun{last}
    {}

    art::EventID id {art::EventID::invalidEvent()};
    bool lastInSubRun {false};
  };

}

#endif /* art_test_Framework_Principal_SimpleEvent_h */

// Local variables:
// mode: c++
// End:
