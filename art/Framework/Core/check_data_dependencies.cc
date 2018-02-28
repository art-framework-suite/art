#include "art/Framework/Core/detail/ModuleGraph.h"
#include "art/Framework/Core/detail/ModuleToPath.h"
#include "art/Framework/Core/detail/graph_algorithms.h"
#include "boost/graph/graph_utility.hpp"

#include <iostream>
#include <fstream>

using art::detail::Edge;
using art::detail::ModuleToPath;
using art::detail::ModuleGraph;
using art::detail::Vertex;
using art::detail::module_name_t;
using art::detail::paths_to_modules_t;

int main()
{
  // Make paths
  paths_to_modules_t paths;
  paths["p1"] = {"a", "b", "c"};
  paths["p2"] = {"f", "d", "e"};
  paths["p3"] = {"f", "g", "h"};
  ModuleToPath const mod_to_path{paths};

  // Data dependencies
  std::map<module_name_t, std::set<module_name_t>> dependencies;
  dependencies["a"] = {"c"};
  dependencies["e"] = {"a", "b"};
  dependencies["c"] = {"f"};
  dependencies["d"] = {"h"};

  auto const module_graph = art::detail::make_module_graph(mod_to_path,
                                                           paths,
                                                           dependencies);
  std::string err;
  err += verify_no_interpath_dependencies(mod_to_path, module_graph);
  err += verify_no_circular_dependencies(mod_to_path, module_graph);
  if (!err.empty()) {
    std::cout << err << '\n';
  }

  bool const verbose{true};
  if (verbose) {
    std::ofstream ofs{"path.dot"};
    print_module_graph(ofs, mod_to_path, module_graph);
  }
}
