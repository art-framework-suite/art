#include "art/Framework/Core/detail/ModuleGraph.h"
#include "art/Framework/Core/detail/ModuleInfoMap.h"
#include "art/Framework/Core/detail/graph_algorithms.h"
#include "boost/graph/graph_utility.hpp"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/make_ParameterSet.h"
#include "fhiclcpp/types/DelegatedParameter.h"
#include "fhiclcpp/types/OptionalDelegatedParameter.h"
#include "fhiclcpp/types/OptionalSequence.h"
#include "fhiclcpp/types/Table.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <fstream>

using art::detail::Edge;
using art::detail::ModuleGraph;
using art::detail::ModuleInfo;
using art::detail::ModuleInfoMap;
using art::detail::Vertex;
using art::detail::module_name_t;
using art::detail::name_set_t;
using art::detail::names_t;
using art::detail::path_name_t;
using art::detail::paths_to_modules_t;

namespace {
  struct TopLevelTable {
    struct Source {
    };
    fhicl::Table<Source> source{fhicl::Name{"source"}};
    fhicl::DelegatedParameter physics{fhicl::Name{"physics"}};
    fhicl::OptionalDelegatedParameter outputs{fhicl::Name{"outputs"}};
  };

  struct ModifierModuleConfig {
    fhicl::OptionalSequence<std::string> depends_on{fhicl::Name{"depends_on"}};
  };

  struct ObserverModuleConfig {
    fhicl::OptionalSequence<std::string> depends_on{fhicl::Name{"depends_on"}};
    fhicl::OptionalSequence<std::string> select_events{fhicl::Name{"SelectEvents"}};
  };

  paths_to_modules_t
  get_paths_to_modules(fhicl::ParameterSet const& physics)
  {
    paths_to_modules_t result;
    for (auto const& name : physics.get_names()) {
      if (!physics.is_key_to_sequence(name))
        continue;
      result[name] = physics.get<std::vector<std::string>>(name);
    }
    return result;
  }

  names_t const tables_with_modifiers{"physics.producers", "physics.filters"};
  names_t const tables_with_observers{"physics.analyzers", "outputs"};

  inline bool
  module_found_in_table(std::string const& module_name,
                        fhicl::ParameterSet const& pset,
                        std::string const& table_name)
  {
    if (!pset.has_key(table_name)) {
      return false;
    }
    auto const& table = pset.get<fhicl::ParameterSet>(table_name);
    return table.has_key(module_name);
  }

  inline std::string
  module_found_with_type(std::string const& module_name,
                         fhicl::ParameterSet const& pset)
  {
    if (module_found_in_table(module_name, pset, "physics.producers"))
      return "producer";
    if (module_found_in_table(module_name, pset, "physics.filters"))
      return "filter";
    if (module_found_in_table(module_name, pset, "physics.analyzers"))
      return "analyzer";
    if (module_found_in_table(module_name, pset, "outputs"))
      return "output";
    return {};
  }

  inline std::string
  table_for_module_type(std::string const& module_type)
  {
    if (module_type == "producer")
      return "physics.producers";
    if (module_type == "filter")
      return "physics.filters";
    if (module_type == "analyzer")
      return "physics.analyzers";
    if (module_type == "output")
      return "outputs";
    return {};
  }

  bool
  module_found_in_tables(std::string const& module_name,
                         fhicl::ParameterSet const& pset,
                         names_t const& table_names)
  {
    for (auto const& table_name : table_names) {
      if (module_found_in_table(module_name, pset, table_name)) {
        return true;
      }
    }
    return false;
  }

  paths_to_modules_t
  select_paths(fhicl::ParameterSet const& pset,
               names_t const& tables,
               paths_to_modules_t& paths_to_modules)
  {
    paths_to_modules_t result;
    auto it = cbegin(paths_to_modules);
    auto e = cend(paths_to_modules);
    for (; it != e; ++it) {
      auto const& path_name = it->first;
      auto const& modules = it->second;

      bool first_module{true};
      bool present{true};
      for (auto const& module_name : modules) {
        if (first_module) {
          first_module = false;
          present = module_found_in_tables(module_name, pset, tables);
        } else if (present !=
                   module_found_in_tables(module_name, pset, tables)) {
          // The presence of the first module determines what the
          // remaining modules should be.
          throw art::Exception{art::errors::LogicError}
            << "There is an inconsistency in path " << path_name << ".\n"
            << "Module " << module_name
            << " is a modifier/observer whereas the other modules\n"
            << "on the path are the opposite.";
        }
      }
      if (present) {
        result.insert(cend(result), *it);
        paths_to_modules.erase(it);
      }
    }
    return result;
  }

  names_t
  merge_end_paths(paths_to_modules_t const& paths_to_modules)
  {
    names_t result;
    for (auto const& pr : paths_to_modules) {
      result.insert(cend(result), cbegin(pr.second), cend(pr.second));
    }
    return result;
  }

  name_set_t
  path_names(paths_to_modules_t const& paths_to_modules)
  {
    name_set_t result;
    for (auto const& pr : paths_to_modules) {
      result.insert(pr.first);
    }
    return result;
  }
}

int
main(int argc, char** argv) try {
  if (argc != 2)
    return 1;

  std::string const filename{argv[1]};

  fhicl::ParameterSet pset;
  cet::filepath_maker maker{};
  make_ParameterSet(filename, maker, pset);
  fhicl::Table<TopLevelTable> table{pset};

  auto const physics = table().physics.get<fhicl::ParameterSet>();
  auto paths_to_modules = get_paths_to_modules(physics);
  auto const trigger_paths =
    select_paths(pset, tables_with_modifiers, paths_to_modules);
  auto end_paths = paths_to_modules;

  auto const end_path = merge_end_paths(end_paths);

  // Get modules
  art::detail::collection_map_t modules{};
  if (!trigger_paths.empty()) {
    modules["*source*"] = ModuleInfo{"source", {}, path_names(trigger_paths)};
  } else if (!end_path.empty()) {
    modules["*source*"] = ModuleInfo{"source", {}, {"end_path"}};
  }

  auto fill_module_info = [&modules](fhicl::ParameterSet const& pset,
                                     std::string const& path_name,
                                     auto const& module_names) {
    for (auto const& module_name : module_names) {
      auto& info = modules[module_name];
      info.paths.insert(path_name);
      info.module_type = module_found_with_type(module_name, pset);
      if (info.module_type == "producer" || info.module_type == "filter") {
        auto const table_name = table_for_module_type(info.module_type);
        auto const& table =
          pset.get<fhicl::ParameterSet>(table_name + "." + module_name);
        fhicl::Table<ModifierModuleConfig> mod{table};
        std::vector<std::string> deps;
        if (mod().depends_on(deps)) {
          info.product_dependencies =
            std::set<std::string>(cbegin(deps), cend(deps));
        }
      } else {
        auto const table_name = table_for_module_type(info.module_type);
        auto const& table =
          pset.get<fhicl::ParameterSet>(table_name + "." + module_name);
        fhicl::Table<ObserverModuleConfig> mod{table};
        std::vector<std::string> deps;
        if (mod().depends_on(deps)) {
          info.product_dependencies =
            std::set<std::string>(cbegin(deps), cend(deps));
        }
        std::vector<std::string> sel;
        if (mod().select_events(sel)) {
          info.select_events = std::set<std::string>(cbegin(sel), cend(sel));
        }
      }
    }
  };

  for (auto const& path : trigger_paths) {
    fill_module_info(pset, path.first, path.second);
  }

  if (!trigger_paths.empty()) {
    modules["TriggerResults"] = ModuleInfo{"producer"};
  }

  fill_module_info(pset, "end_path", end_path);

  ModuleInfoMap const modInfos{modules};
  auto const module_graph = art::detail::make_module_graph(modInfos,
                                                           trigger_paths,
                                                           end_path);
  auto const& err = module_graph.second;
  if (!err.empty()) {
    std::cout << err << '\n';
  }

  auto const pos = filename.find(".fcl");
  std::string const basename =
    (pos != std::string::npos) ? filename.substr(0, pos) : filename;
  std::ofstream ofs{basename + ".dot"};
  print_module_graph(ofs, modInfos, module_graph.first);
}
catch (fhicl::detail::validationException const& v) {
  std::cerr << v.what();
}
catch (std::exception const& e) {
  std::cerr << e.what() << '\n';
}
catch (...) {
  std::cerr << "Job failed.\n";
}
