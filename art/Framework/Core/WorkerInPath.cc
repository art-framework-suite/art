// ======================================================================
//
// WorkerInPath: A wrapper around a Worker, so that statistics can be
//               managed per path.  A Path holds Workers as these things.
//
// ======================================================================

#include "art/Framework/Core/WorkerInPath.h"

using art::WorkerInPath;

WorkerInPath::WorkerInPath(Worker * w, FilterAction theFilterAction)
  : stopwatch_(new RunStopwatch::StopwatchPointer::element_type)
  , timesVisited_(0)
  , timesPassed_(0)
  , timesFailed_(0)
  , timesExcept_(0)
  , filterAction_(theFilterAction)
  , worker_(w)
{ }

WorkerInPath::WorkerInPath(Worker * w)
  : stopwatch_(new RunStopwatch::StopwatchPointer::element_type)
  , timesVisited_(0)
  , timesPassed_(0)
  , timesFailed_(0)
  , timesExcept_(0)
  , filterAction_(Normal)
  , worker_(w)
{ }
