#ifndef art_Framework_Core_WorkerMap_h
#define art_Framework_Core_WorkerMap_h

#include "art/Framework/Principal/Worker.h"

#include <map>
#include <memory>

namespace art {
  typedef std::map<std::string, std::shared_ptr<Worker>> WorkerMap;
}
#endif /* art_Framework_Core_WorkerMap_h */

// Local Variables:
// mode: c++
// End:
