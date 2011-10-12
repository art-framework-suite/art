#ifndef art_Utilities_CPUTimer_h
#define art_Utilities_CPUTimer_h

#include "art/Utilities/fwd.h"

#include <sys/time.h>

namespace art {
  class CPUTimer {

  public:
    CPUTimer();
    virtual ~CPUTimer();

    // ---------- const member functions ---------------------
    double realTime() const ;
    double cpuTime() const ;

    // ---------- static member functions --------------------

    // ---------- member functions ---------------------------
    void start();
    void stop();
    void reset();

  private:
    CPUTimer(const CPUTimer &); // stop default

    const CPUTimer & operator=(const CPUTimer &); // stop default

    struct Times {
      double real_;
      double cpu_;
    };

    Times calculateDeltaTime() const;

    // ---------- member data --------------------------------
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
