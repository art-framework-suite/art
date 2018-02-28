#ifndef art_Framework_Core_detail_graph_algorithms_h
#define art_Framework_Core_detail_graph_algorithms_h

#include "art/Framework/Core/detail/ModuleGraph.h"
#include "art/Framework/Core/detail/ModuleInfoMap.h"

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace art {
  namespace detail {

    // Make subgraphs - one per path
    void make_edges_path_orderings(ModuleInfoMap const& modInfos,
                                   paths_to_modules_t const& paths,
                                   ModuleGraph& graph);

    void make_synchronization_edges(ModuleInfoMap const& modInfos,
                                    paths_to_modules_t const& trigger_paths,
                                    names_t const& end_path,
                                    ModuleGraph& graph);

    // Make edges for implicit module dependencies ("consumes" statements)
    std::string verify_no_interpath_dependencies(ModuleInfoMap const& modInfos,
                                                 ModuleGraph const& graph);

    std::string verify_in_order_dependencies(
      ModuleInfoMap const& modules,
      paths_to_modules_t const& trigger_paths);

    void print_module_graph(std::ostream& os,
                            ModuleInfoMap const& modInfos,
                            ModuleGraph const& graph);
  }
}

#endif
