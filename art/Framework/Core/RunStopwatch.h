#ifndef art_Framework_Core_RunStopwatch_h
#define art_Framework_Core_RunStopwatch_h

/*----------------------------------------------------------------------



Simple "guard" class as suggested by Chris Jones to start/stop the
Stopwatch: creating an object of type RunStopwatch starts the clock
pointed to, deleting it (when it goes out of scope) automatically
calls the destructor which stops the clock.

----------------------------------------------------------------------*/

#include "cpp0x/memory"
#include "art/Utilities/CPUTimer.h"

namespace art {

  class RunStopwatch {

  public:
    typedef std::shared_ptr<CPUTimer> StopwatchPointer;

    RunStopwatch(const StopwatchPointer& ptr): stopwatch_(ptr) {
      stopwatch_->start();
    }

    ~RunStopwatch(){
      stopwatch_->stop();
    }

  private:
    StopwatchPointer stopwatch_;

  };

}
#endif /* art_Framework_Core_RunStopwatch_h */

// Local Variables:
// mode: c++
// End:
