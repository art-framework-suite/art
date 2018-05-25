#include "art/Framework/Core/detail/ModuleGraph.h"
#include "art/Framework/Core/detail/ModuleGraphInfoMap.h"
#include "art/Framework/Core/detail/graph_algorithms.h"
#include "boost/graph/graph_utility.hpp"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/make_ParameterSet.h"
#include "fhiclcpp/types/OptionalAtom.h"
#include "fhiclcpp/types/OptionalDelegatedParameter.h"
#include "fhiclcpp/types/OptionalSequence.h"
#include "fhiclcpp/types/OptionalTable.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/TupleAs.h"

#include <cassert>
#include <fstream>
#include <iostream>

using namespace art::detail;
using namespace fhicl;
using namespace std::string_literals;
using std::string;

namespace {
  void
  throwIfEmpty(string const& friendlyName)
  {
    if (friendlyName.empty()) {
      throw art::Exception{art::errors::Configuration,
                           "There was an error processing typenames.\n"}
        << "No friendly type name was provided.\n";
    }
  }

  // For producing products
  struct TypeAndInstance {
    explicit TypeAndInstance(string friendlyName, string instance)
      : friendlyClassName{(throwIfEmpty(friendlyName), move(friendlyName))}
      , productInstanceName{move(instance)}
    {}

    string friendlyClassName;
    string productInstanceName;
  };

  // For consuming products
  struct TypeAndTag {
    explicit TypeAndTag(string friendlyName, art::InputTag tag)
      : friendlyClassName{(throwIfEmpty(friendlyName), move(friendlyName))}
      , inputTag{std::move(tag)}
    {}

    string friendlyClassName;
    art::InputTag inputTag;
  };

  struct TopLevelTable {
    struct TestProperties {
      Atom<bool> graph_failure_expected{Name{"graph_failure_expected"}};
      OptionalAtom<string> error_message{Name{"error_message"}};
    };
    Table<TestProperties> test_properties{Name{"test_properties"}};
    Atom<string> process_name{Name{"process_name"}};
    struct Source {
      Atom<string> module_type{Name{"module_type"}};
    };
    OptionalTable<Source> source{Name{"source"}};
    OptionalDelegatedParameter physics{Name{"physics"}};
    OptionalDelegatedParameter outputs{Name{"outputs"}};
  };

  struct ModifierModuleConfig {
    OptionalSequence<TupleAs<TypeAndInstance(string, string)>> produces{
      Name{"produces"}};
    OptionalSequence<TupleAs<TypeAndTag(string, art::InputTag)>> consumes{
      Name{"consumes"}};
    OptionalSequence<string> consumesMany{Name{"consumesMany"}};
  };

  struct ObserverModuleConfig {
    OptionalSequence<TupleAs<TypeAndTag(string, art::InputTag)>> consumes{
      Name{"consumes"}};
    OptionalSequence<string> consumesMany{Name{"consumesMany"}};
    OptionalSequence<string> select_events{Name{"SelectEvents"}};
  };

  paths_to_modules_t
  get_paths_to_modules(ParameterSet const& physics)
  {
    paths_to_modules_t result;
    for (auto const& name : physics.get_names()) {
      if (!physics.is_key_to_sequence(name))
        continue;
      auto const tmp = physics.get<std::vector<string>>(name);
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
  module_found_in_table(string const& module_name,
                        ParameterSet const& pset,
                        string const& table_name)
  {
    if (!pset.has_key(table_name)) {
      return false;
    }
    auto const& table = pset.get<ParameterSet>(table_name);
    return table.has_key(module_name);
  }

  inline art::ModuleType
  module_found_with_type(string const& module_name, ParameterSet const& pset)
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

  inline string
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
  module_found_in_tables(string const& module_name,
                         ParameterSet const& pset,
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
  select_paths(ParameterSet const& pset,
               names_t const& tables,
               paths_to_modules_t& paths_to_modules)
  {
    paths_to_modules_t result;
    std::vector<string> paths_to_erase;
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

  template <typename T>
  std::set<art::ProductInfo>
  sorted_dependencies(Table<T> const& module,
                      string const& current_process_name)
  {
    std::set<art::ProductInfo> sorted_deps;
    std::vector<TypeAndTag> deps;
    if (module().consumes(deps)) {
      for (auto const& dep : deps) {
        art::ProcessTag const processTag{dep.inputTag.process(),
                                         current_process_name};
        // In cases where a user has not specified the current process
        // name (or the literal "current_process"), we set the label
        // of the module this worker depends upon to "input_source",
        // solely for data-dependency checking.  This permits users to
        // specify only a module label in the input tag, and even
        // though this might collide with a module label in the
        // current process, it is not necessarily an error.
        //
        // In the future, we may wish to constrain the behavior so
        // that if there is an ambiguity in module labels between
        // processes, a user will be required to specify
        // "current_process" or "input_source".
        std::string const label = (processTag.name() != current_process_name) ?
                                    "input_source" :
                                    dep.inputTag.label();
        sorted_deps.emplace(art::ProductInfo::ConsumableType::Product,
                            dep.friendlyClassName,
                            label,
                            dep.inputTag.instance(),
                            processTag);
      }
    }
    return sorted_deps;
  }

  void
  fill_module_info(ParameterSet const& pset,
                   string const& process_name,
                   string const& path_name,
                   configs_t const& module_configs,
                   art::detail::collection_map_t& modules)
  {
    for (auto const& config : module_configs) {
      auto const& module_name = config.label;
      auto& info = modules[module_name];
      info.paths.insert(path_name);
      info.module_type = module_found_with_type(module_name, pset);
      auto const table_name = table_for_module_type(info.module_type);
      auto const& table =
        pset.get<ParameterSet>(table_name + "." + module_name);
      if (is_modifier(info.module_type)) {
        Table<ModifierModuleConfig> const mod{table};
        info.product_dependencies = sorted_dependencies(mod, process_name);
      } else {
        Table<ObserverModuleConfig> const mod{table};
        info.product_dependencies = sorted_dependencies(mod, process_name);
        std::vector<string> sel;
        if (mod().select_events(sel)) {
          info.select_events = std::set<string>(cbegin(sel), cend(sel));
        }
      }
    }
  }
}

int
main(int argc, char** argv) try {
  if (argc != 2)
    return 1;

  string const filename{argv[1]};

  ParameterSet pset;
  cet::filepath_maker maker{};
  make_ParameterSet(filename, maker, pset);
  Table<TopLevelTable> table{pset};
  auto const& process_name = table().process_name();
  auto const& test_properties = table().test_properties();

  ParameterSet physics;
  if (!table().physics.get_if_present(physics)) {
    return 0;
  }

  // Form the paths
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

  for (auto const& path : trigger_paths) {
    fill_module_info(pset, process_name, path.first, path.second, modules);
  }

  if (!trigger_paths.empty()) {
    modules["TriggerResults"] = ModuleGraphInfo{art::ModuleType::producer};
  }

  fill_module_info(pset, process_name, "end_path", end_path, modules);

  // Build the graph
  ModuleGraphInfoMap const modInfos{modules};
  auto const module_graph =
    art::detail::make_module_graph(modInfos, trigger_paths, end_path);
  auto const& err = module_graph.second;
  bool graph_failure{false};
  std::ostringstream oss;
  if (!err.empty()) {
    oss << err << '\n';
    graph_failure = true;
  }

  auto const pos = filename.find(".fcl");
  string const basename =
    (pos != string::npos) ? filename.substr(0, pos) : filename;
  std::ofstream ofs{basename + ".dot"};
  print_module_graph(ofs, modInfos, module_graph.first);

  // Check if test properties have been satisfied
  int rc{};
  bool const graph_failure_expected = test_properties.graph_failure_expected();
  if (graph_failure != graph_failure_expected) {
    std::cerr << "Graph failure value is not correct:\n"
              << "    " << std::boolalpha << graph_failure_expected << " (expected) vs. "
              << graph_failure <<  " (actual)\n";
    rc = 1;
  }
  string expected_msg;
  if (test_properties.error_message(expected_msg)) {
    auto const msg = oss.str();
    std::regex const re{expected_msg};
    if (!graph_failure) {
      std::cerr << "An error message was generated even though there was no "
                   "graph failure.\n";
      rc = 2;
    } else if (!std::regex_search(msg, re)) {
      std::cerr << " The error message does not match what was expected:\n"
                << "   Actual: " << msg << '\n'
                << "   Expected: " << expected_msg << '\n';
      rc = 3;
    }
  }
  return rc;
}
catch (detail::validationException const& v) {
  std::cerr << v.what();
  return 1;
}
catch (std::exception const& e) {
  std::cerr << e.what() << '\n';
  return 1;
}
catch (...) {
  std::cerr << "Job failed.\n";
  return 1;
}
