#ifndef art_Framework_Core_WorkerMap_h
#define art_Framework_Core_WorkerMap_h
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Worker.h"

#include <map>
#include <memory>

namespace art {
using WorkerMap = std::map<std::string, std::unique_ptr<Worker>>;
}
#endif /* art_Framework_Core_WorkerMap_h */

// Local Variables:
// mode: c++
// End:
