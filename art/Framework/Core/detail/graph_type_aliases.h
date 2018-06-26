#ifndef art_Framework_Core_detail_graph_type_aliases_h
#define art_Framework_Core_detail_graph_type_aliases_h

#include "art/Framework/Core/WorkerInPath.h"
#include "art/Framework/Core/detail/ModuleGraphInfo.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace art {
  namespace detail {
    using path_name_t = std::string;
    using module_name_t = std::string;
    using names_t = std::vector<std::string>;
    using configs_t = std::vector<WorkerInPath::ConfigInfo>;
    using name_set_t = std::set<std::string>;
    using paths_to_modules_t = std::map<path_name_t, configs_t>;
    using collection_map_t = std::map<module_name_t, ModuleGraphInfo>;
    using collection_t = std::vector<collection_map_t::value_type>;
    using distance_t = collection_t::difference_type;
  }
}

#endif /* art_Framework_Core_detail_graph_type_aliases_h */

// Local Variables:
// mode: c++
// End:
