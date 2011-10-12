//
// Package:     Utilities
// Class  :     CPUTimer
//

#include "art/Utilities/CPUTimer.h"
#include <sys/resource.h>
#include <errno.h>
#include "cetlib/exception.h"

using namespace art;

CPUTimer::CPUTimer() :
  state_(kStopped),
  startRealTime_(),
  startCPUTime_(),
  accumulatedRealTime_(0),
  accumulatedCPUTime_(0)
{
  startRealTime_.tv_sec = 0;
  startRealTime_.tv_usec = 0;
  startCPUTime_.tv_sec = 0;
  startCPUTime_.tv_usec = 0;
}

CPUTimer::~CPUTimer()
{
}

void
CPUTimer::start()
{
  if (kStopped == state_) {
    rusage theUsage;
    if (0 != getrusage(RUSAGE_SELF, &theUsage)) {
      throw cet::exception("CPUTimerFailed") << errno;
    }
    startCPUTime_.tv_sec = theUsage.ru_stime.tv_sec + theUsage.ru_utime.tv_sec;
    startCPUTime_.tv_usec = theUsage.ru_stime.tv_usec + theUsage.ru_utime.tv_usec;
    gettimeofday(&startRealTime_, 0);
    state_ = kRunning;
  }
}

void
CPUTimer::stop()
{
  if (kRunning == state_) {
    Times t = calculateDeltaTime();
    accumulatedCPUTime_ += t.cpu_;
    accumulatedRealTime_ += t.real_;
    state_ = kStopped;
  }
}

void
CPUTimer::reset()
{
  accumulatedCPUTime_ = 0;
  accumulatedRealTime_ = 0;
}

CPUTimer::Times
CPUTimer::calculateDeltaTime() const
{
  rusage theUsage;
  if (0 != getrusage(RUSAGE_SELF, &theUsage)) {
    throw cet::exception("CPUTimerFailed") << errno;
  }
  const double microsecToSec = 1E-6;
  struct timeval tp;
  gettimeofday(&tp, 0);
  Times returnValue;
  returnValue.cpu_ = theUsage.ru_stime.tv_sec + theUsage.ru_utime.tv_sec - startCPUTime_.tv_sec + microsecToSec * (theUsage.ru_stime.tv_usec + theUsage.ru_utime.tv_usec - startCPUTime_.tv_usec);
  returnValue.real_ = tp.tv_sec - startRealTime_.tv_sec + microsecToSec * (tp.tv_usec - startRealTime_.tv_usec);
  return returnValue;
}

double
CPUTimer::realTime() const
{
  if (kStopped == state_) {
    return accumulatedRealTime_;
  }
  return accumulatedRealTime_ + calculateDeltaTime().real_;
}

double
CPUTimer::cpuTime() const
{
  if (kStopped == state_) {
    return accumulatedCPUTime_;
  }
  return accumulatedCPUTime_ + calculateDeltaTime().cpu_;
}
