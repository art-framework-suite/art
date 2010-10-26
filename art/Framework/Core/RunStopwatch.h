#ifndef FWCore_Framework_RunStopwatch_h
#define FWCore_Framework_RunStopwatch_h

/*----------------------------------------------------------------------



Simple "guard" class as suggested by Chris Jones to start/stop the
Stopwatch: creating an object of type RunStopwatch starts the clock
pointed to, deleting it (when it goes out of scope) automatically
calls the destructor which stops the clock.

----------------------------------------------------------------------*/

#include "boost/shared_ptr.hpp"
#include "art/Utilities/CPUTimer.h"

namespace art {

  class RunStopwatch {

  public:
    typedef boost::shared_ptr<CPUTimer> StopwatchPointer;

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
#endif
