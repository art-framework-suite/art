#ifndef art_Framework_Principal_MaybeRunStopwatch_h
#define art_Framework_Principal_MaybeRunStopwatch_h

/*----------------------------------------------------------------------

Simple "guard" class as suggested by Chris Jones to start/stop the
Stopwatch: creating an object of type MaybeRunStopwatch starts the clock
pointed to, deleting it (when it goes out of scope) automatically
calls the destructor which stops the clock.

----------------------------------------------------------------------*/

#include "cetlib/cpu_timer.h"

namespace art {

  struct Stopwatch {
    using timer_type = cet::cpu_timer;
  };

  template <bool run>
  class MaybeMaybeRunStopwatch : Stopwatch {
  public:

    MaybeMaybeRunStopwatch(timer_type& timer): stopwatch_{timer} {
      stopwatch_.start();
    }

    ~MaybeMaybeRunStopwatch() {
      stopwatch_.stop();
    }

  private:
    timer_type& stopwatch_;
  };

  template <>
  class MaybeMaybeRunStopwatch<false> : Stopwatch {
  public:
    using timer_type = cet::cpu_timer;
    MaybeMaybeRunStopwatch(timer_type&){}
  };

}

#endif /* art_Framework_Principal_MaybeRunStopwatch_h */

// Local Variables:
// mode: c++
// End:
