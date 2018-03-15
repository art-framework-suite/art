#ifndef art_Framework_Core_detail_graph_algorithms_h
#define art_Framework_Core_detail_graph_algorithms_h

#include "art/Framework/Core/detail/ModuleGraph.h"
#include "art/Framework/Core/detail/ModuleGraphInfoMap.h"

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace art {
  namespace detail {

    std::pair<ModuleGraph, std::string>
    make_module_graph(ModuleGraphInfoMap const& modInfos,
                      paths_to_modules_t const& trigger_paths,
                      configs_t const& end_path);

    // Make subgraphs - one per path
    void make_trigger_path_subgraphs(ModuleGraphInfoMap const& modInfos,
                                     paths_to_modules_t const& trigger_paths,
                                     ModuleGraph& graph);

    void make_product_dependency_edges(ModuleGraphInfoMap const& modInfos,
                                       ModuleGraph& graph);

    void make_path_ordering_edges(ModuleGraphInfoMap const& modInfos,
                                  paths_to_modules_t const& paths,
                                  ModuleGraph& graph);

    void make_synchronization_edges(ModuleGraphInfoMap const& modInfos,
                                    paths_to_modules_t const& trigger_paths,
                                    configs_t const& end_path,
                                    ModuleGraph& graph);

    // Make edges for implicit module dependencies ("consumes" statements)
    std::string verify_no_interpath_dependencies(ModuleGraphInfoMap const& modInfos,
                                                 ModuleGraph const& graph);

    std::string verify_in_order_dependencies(
      ModuleGraphInfoMap const& modules,
      paths_to_modules_t const& trigger_paths);

    void print_module_graph(std::ostream& os,
                            ModuleGraphInfoMap const& modInfos,
                            ModuleGraph const& graph);
  }
}

#endif
