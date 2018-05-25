#include "art/Framework/Core/detail/ModuleGraph.h"
#include "art/Framework/Core/detail/ModuleGraphInfoMap.h"
#include "art/Framework/Core/detail/graph_algorithms.h"
#include "boost/graph/graph_utility.hpp"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/make_ParameterSet.h"
#include "fhiclcpp/types/OptionalDelegatedParameter.h"
#include "fhiclcpp/types/OptionalSequence.h"
#include "fhiclcpp/types/OptionalTable.h"
#include "fhiclcpp/types/Table.h"

#include <cassert>
#include <fstream>
#include <iostream>

using namespace art::detail;
using namespace std::string_literals;

namespace {
  struct TopLevelTable {
    struct Source {
    };
    fhicl::OptionalTable<Source> source{fhicl::Name{"source"}};
    fhicl::OptionalDelegatedParameter physics{fhicl::Name{"physics"}};
    fhicl::OptionalDelegatedParameter outputs{fhicl::Name{"outputs"}};
  };

  struct ModifierModuleConfig {
    fhicl::OptionalSequence<std::string> depends_on{fhicl::Name{"depends_on"}};
  };

  struct ObserverModuleConfig {
    fhicl::OptionalSequence<std::string> depends_on{fhicl::Name{"depends_on"}};
    fhicl::OptionalSequence<std::string> select_events{
      fhicl::Name{"SelectEvents"}};
  };

  paths_to_modules_t
  get_paths_to_modules(fhicl::ParameterSet const& physics)
  {
    paths_to_modules_t result;
    for (auto const& name : physics.get_names()) {
      if (!physics.is_key_to_sequence(name))
        continue;
      auto const tmp = physics.get<std::vector<std::string>>(name);
      configs_t configs;
      cet::transform_all(tmp, back_inserter(configs), [](auto const& str) {
        return art::WorkerInPath::ConfigInfo{str, art::WorkerInPath::Normal};
      });
      result[name] = configs;
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

  inline art::ModuleType
  module_found_with_type(std::string const& module_name,
                         fhicl::ParameterSet const& pset)
  {
    if (module_found_in_table(module_name, pset, "physics.producers"))
      return art::ModuleType::producer;
    if (module_found_in_table(module_name, pset, "physics.filters"))
      return art::ModuleType::filter;
    if (module_found_in_table(module_name, pset, "physics.analyzers"))
      return art::ModuleType::analyzer;
    if (module_found_in_table(module_name, pset, "outputs"))
      return art::ModuleType::output_module;
    return art::ModuleType::non_art;
  }

  inline std::string
  table_for_module_type(art::ModuleType const module_type)
  {
    if (module_type == art::ModuleType::producer)
      return "physics.producers";
    if (module_type == art::ModuleType::filter)
      return "physics.filters";
    if (module_type == art::ModuleType::analyzer)
      return "physics.analyzers";
    if (module_type == art::ModuleType::output_module)
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
    std::vector<std::string> paths_to_erase;
    for (auto const& pr : paths_to_modules) {
      auto const& path_name = pr.first;
      auto const& modules = pr.second;

      bool first_module{true};
      bool present{true};
      for (auto const& module : modules) {
        if (first_module) {
          first_module = false;
          present = module_found_in_tables(module.label, pset, tables);
        } else if (present !=
                   module_found_in_tables(module.label, pset, tables)) {
          // The presence of the first module determines what the
          // remaining modules should be.
          throw art::Exception{art::errors::LogicError}
            << "There is an inconsistency in path " << path_name << ".\n"
            << "Module " << module.label
            << " is a modifier/observer whereas the other modules\n"
            << "on the path are the opposite.";
        }
      }
      if (present) {
        result.insert(cend(result), pr);
        paths_to_erase.push_back(path_name);
      }
    }
    for (auto const& path : paths_to_erase) {
      paths_to_modules.erase(path);
    }
    return result;
  }

  configs_t
  merge_end_paths(paths_to_modules_t const& paths_to_modules)
  {
    configs_t result;
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

  fhicl::ParameterSet physics;
  if (!table().physics.get_if_present(physics)) {
    return 0;
  }
  auto paths_to_modules = get_paths_to_modules(physics);
  auto const trigger_paths =
    select_paths(pset, tables_with_modifiers, paths_to_modules);
  auto end_paths = paths_to_modules;

  auto const end_path = merge_end_paths(end_paths);

  // Get modules
  art::detail::collection_map_t modules{};

  auto& source_info = modules["input_source"];
  if (!trigger_paths.empty()) {
    source_info.paths = path_names(trigger_paths);
  } else if (!end_path.empty()) {
    source_info.paths = {"end_path"};
  }

  auto fill_module_info = [&modules](fhicl::ParameterSet const& pset,
                                     std::string const& path_name,
                                     configs_t const& module_configs) {
    for (auto const& config : module_configs) {
      auto const& module_name = config.label;
      auto& info = modules[module_name];
      info.paths.insert(path_name);
      info.module_type = module_found_with_type(module_name, pset);
      if (is_modifier(info.module_type)) {
        auto const table_name = table_for_module_type(info.module_type);
        auto const& table =
          pset.get<fhicl::ParameterSet>(table_name + "." + module_name);
        fhicl::Table<ModifierModuleConfig> mod{table};
        std::vector<std::string> deps;
        if (mod().depends_on(deps)) {
          // Assume all the same type for simplicity...for now
          std::set<art::ProductInfo> sorted_deps;
          for (auto const& module_label : deps ) {
            sorted_deps.emplace(art::ProductInfo::ConsumableType::Product,
                                art::TypeID{typeid(int)},
                                module_label,
                                ""s,
                                art::ProcessTag{});
          }
          info.product_dependencies = move(sorted_deps);
        }
      } else {
        auto const table_name = table_for_module_type(info.module_type);
        auto const& table =
          pset.get<fhicl::ParameterSet>(table_name + "." + module_name);
        fhicl::Table<ObserverModuleConfig> mod{table};
        std::vector<std::string> deps;
        if (mod().depends_on(deps)) {
          // Assume all the same type for simplicity...for now
          std::set<art::ProductInfo> sorted_deps;
          for (auto const& module_label : deps ) {
            sorted_deps.emplace(art::ProductInfo::ConsumableType::Product,
                                art::TypeID{typeid(int)},
                                module_label,
                                ""s,
                                art::ProcessTag{});
          }
          info.product_dependencies = move(sorted_deps);
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
    modules["TriggerResults"] = ModuleGraphInfo{art::ModuleType::producer};
  }

  fill_module_info(pset, "end_path", end_path);

  ModuleGraphInfoMap const modInfos{modules};
  auto const module_graph =
    art::detail::make_module_graph(modInfos, trigger_paths, end_path);
  auto const& err = module_graph.second;
  int rc{};
  if (!err.empty()) {
    std::cout << err << '\n';
    rc = 1;
  }

  auto const pos = filename.find(".fcl");
  std::string const basename =
    (pos != std::string::npos) ? filename.substr(0, pos) : filename;
  std::ofstream ofs{basename + ".dot"};
  print_module_graph(ofs, modInfos, module_graph.first);
  return rc;
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
