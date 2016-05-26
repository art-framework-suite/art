// ======================================================================
//
// WorkerInPath: A wrapper around a Worker, so that statistics can be
//               managed per path.  A Path holds Workers as these things.
//
// ======================================================================

#include "art/Framework/Core/WorkerInPath.h"

using art::WorkerInPath;

WorkerInPath::WorkerInPath(Worker* w, FilterAction const theFilterAction)
  : filterAction_{theFilterAction}
  , worker_{w}
{ }

WorkerInPath::WorkerInPath(Worker* w)
  : worker_{w}
{ }
