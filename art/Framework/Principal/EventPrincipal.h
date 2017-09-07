#ifndef art_Framework_Principal_EventPrincipal_h
#define art_Framework_Principal_EventPrincipal_h
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/NoDelayedReader.h"
#include "art/Framework/Principal/Principal.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/History.h"
#include "cetlib/exempt_ptr.h"

#include <memory>

namespace art {

class ProcessConfiguration;

class EventPrincipal final : public Principal {

public:

  using Auxiliary = EventAuxiliary;
  static constexpr BranchType branch_type = Auxiliary::branch_type;

public:

  ~EventPrincipal();

  EventPrincipal(EventAuxiliary const& aux,
                 ProcessConfiguration const& pc,
                 std::unique_ptr<History>&& history = std::make_unique<History>(),
                 std::unique_ptr<DelayedReader>&& rtrv = std::make_unique<NoDelayedReader>(),
                 bool lastInSubRun = false);

};

} // namespace art

// Local Variables:
// mode: c++
// End:

#endif /* art_Framework_Principal_EventPrincipal_h */
