#include "art/Framework/Core/PathsInfo.h"

art::PathsInfo::
PathsInfo()
 :
  workers_(),
  pathPtrs_(),
  pathResults_(),
  totalEvents_(0),
  passedEvents_(0),
  stopwatch_(new RunStopwatch::StopwatchPointer::element_type)
{
}
