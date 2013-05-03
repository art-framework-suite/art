#ifndef art_Utilities_CPUTimer_h
#define art_Utilities_CPUTimer_h

#include "art/Utilities/fwd.h"

namespace art

{
  class CPUTimer
  {
  public:
    CPUTimer();
    CPUTimer(const CPUTimer&) = delete;
    CPUTimer& operator=(const CPUTimer&) = delete;

    // Return time in seconds.
    double realTime() const;
    double cpuTime() const;

    void start();
    void stop();
    void reset();

  private:

    struct Times {
      double real_;
      double cpu_;
    };

    Times calculateDeltaTime() const;

    enum State {kRunning, kStopped} state_;
    struct timeval startRealTime_;
    struct timeval startCPUTime_;

    double accumulatedRealTime_;
    double accumulatedCPUTime_;
  };
}

#endif /* art_Utilities_CPUTimer_h */

// Local Variables:
// mode: c++
// End:
