#ifndef art_Framework_Core_PathsInfo_h
#define art_Framework_Core_PathsInfo_h
#include "art/Framework/Core/Path.h"
#include "art/Framework/Core/detail/WorkerMap.h"
#include "art/Persistency/Common/HLTGlobalStatus.h"

namespace art {
  class PathsInfo;
}

struct art::PathsInfo {
  WorkerMap workers;
  PathPtrs pathPtrs;
  HLTGlobalStatus pathResults;
};

#endif /* art_Framework_Core_PathsInfo_h */

// Local Variables:
// mode: c++
// End:
