#ifndef art_Framework_Core_detail_graph_algorithms_h
#define art_Framework_Core_detail_graph_algorithms_h

#include "art/Framework/Core/detail/ModuleGraph.h"
#include "art/Framework/Core/detail/ModuleToPath.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace art {
  namespace detail {
    using path_name_t = std::string;
    using module_name_t = std::string;
    using names_t = std::vector<std::string>;
    using paths_to_modules_t = std::map<path_name_t, names_t>;
    using modules_to_paths_t = std::vector<std::pair<module_name_t, names_t>>;

    ModuleGraph make_module_graph(ModuleToPath const& mod_to_path,
                                  paths_to_modules_t const& paths,
                                  std::map<std::string, std::set<std::string>> const& deps);

    // Make subgraphs - one per path
    void make_path_graphs(ModuleToPath const& mod_to_path,
                          paths_to_modules_t const& paths,
                          ModuleGraph& graph);

    void make_edges_path_orderings(ModuleToPath const& mod_to_path,
                                   paths_to_modules_t const& paths,
                                   ModuleGraph& graph);

    // Make edges for implicit module dependencies ("consumes" statements)
    void make_edges_data_dependencies(ModuleToPath const& mod_to_path,
                                      std::map<std::string, std::set<std::string>> const& deps,
                                      ModuleGraph& graph);

    std::string
    verify_no_interpath_dependencies(ModuleToPath const& mod_to_path,
                                     ModuleGraph const& graph);

    std::string
    verify_no_circular_dependencies(ModuleToPath const& mod_to_path,
                                    ModuleGraph const& graph);

    void
    print_module_graph(std::ostream& os,
                       ModuleToPath const& mod_to_path,
                       ModuleGraph const& graph);
  }
}

#endif
