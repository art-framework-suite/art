#ifndef art_Framework_Principal_MaybeRunStopwatch_h
#define art_Framework_Principal_MaybeRunStopwatch_h

/*----------------------------------------------------------------------

Simple "guard" class as suggested by Chris Jones to start/stop the
Stopwatch: creating an object of type MaybeRunStopwatch starts the clock
pointed to, deleting it (when it goes out of scope) automatically
calls the destructor which stops the clock.

----------------------------------------------------------------------*/

#include "canvas/Persistency/Provenance/IDNumber.h"
#include "cetlib/cpu_timer.h"

namespace art {

  struct Stopwatch {
    using timer_type = cet::cpu_timer;
  };

  template <Level>
  class MaybeRunStopwatch : Stopwatch {
    using timer_type = cet::cpu_timer;
  public:
    MaybeRunStopwatch(timer_type&){}
  };

  template <>
  class MaybeRunStopwatch<Level::Event> : Stopwatch {
  public:

    MaybeRunStopwatch(timer_type& timer): stopwatch_{timer} {
      stopwatch_.start();
    }

    ~MaybeRunStopwatch() {
      stopwatch_.stop();
    }

  private:
    timer_type& stopwatch_;
  };

}

#endif /* art_Framework_Principal_MaybeRunStopwatch_h */

// Local Variables:
// mode: c++
// End:
