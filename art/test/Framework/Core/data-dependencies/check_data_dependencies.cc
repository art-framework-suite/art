#include "art/Framework/Core/detail/ModuleConfigInfo.h"
#include "art/Framework/Core/detail/ModuleGraph.h"
#include "art/Framework/Core/detail/ModuleGraphInfoMap.h"
#include "art/Framework/Core/detail/consumed_products.h"
#include "art/Framework/Core/detail/graph_algorithms.h"
#include "art/Framework/Principal/ConsumesInfo.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/test/Framework/Core/data-dependencies/Configs.h"
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
  names_t const all_tables{"physics.producers",
                           "physics.filters",
                           "physics.analyzers",
                           "outputs"};
  names_t const tables_with_modifiers{all_tables[0], all_tables[1]};
  names_t const tables_with_observers{all_tables[2], all_tables[3]};

  art::ModuleType
  module_type_for_table(string const& table)
  {
    if (table == "physics.producers")
      return art::ModuleType::producer;
    if (table == "physics.filters")
      return art::ModuleType::filter;
    if (table == "physics.analyzers")
      return art::ModuleType::analyzer;
    if (table == "outputs")
      return art::ModuleType::output_module;
    return art::ModuleType::non_art;
  }

  auto
  get_module_configs(ParameterSet const& pset)
  {
    std::map<std::string, ModuleConfigInfo> result;
    for (auto const& name : all_tables) {
      if (!pset.has_key(name))
        continue;

      if (!pset.is_key_to_table(name))
        continue;

      auto const table_pset = pset.get<fhicl::ParameterSet>(name);
      auto const modules_in_table = table_pset.get_pset_names();
      auto const module_type = module_type_for_table(name);
      for (auto const& module_name : modules_in_table) {
        art::ModuleDescription const md{
          {},
          module_name, // Use module-name for libspec
          module_name,
          art::ModuleThreadingType::shared,
          art::ProcessConfiguration{}};
        ModuleConfigInfo info{
          md, table_pset.get<fhicl::ParameterSet>(module_name), module_type};
        result.emplace(module_name, std::move(info));
      }
    }
    return result;
  }

  paths_to_modules_t
  get_paths_to_modules(
    ParameterSet const& physics,
    std::map<std::string, ModuleConfigInfo> const& module_configs)
  {
    paths_to_modules_t result;
    for (auto const& name : physics.get_names()) {
      if (!physics.is_key_to_sequence(name))
        continue;
      auto const tmp = physics.get<std::vector<string>>(name);
      configs_t configs;
      cet::transform_all(
        tmp, back_inserter(configs), [&module_configs](auto const& label) {
          auto const& info = module_configs.at(label);
          return art::WorkerInPath::ConfigInfo{
            cet::make_exempt_ptr(&info), art::detail::FilterAction::Normal};
        });
      result.emplace_back(name, std::move(configs));
    }
    return result;
  }

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
        auto const& module_label =
          module.moduleConfigInfo->modDescription.moduleLabel();
        if (first_module) {
          first_module = false;
          present = module_found_in_tables(module_label, pset, tables);
        } else if (present !=
                   module_found_in_tables(module_label, pset, tables)) {
          // The presence of the first module determines what the
          // remaining modules should be.
          throw art::Exception{art::errors::LogicError}
            << "There is an inconsistency in path " << path_name << ".\n"
            << "Module " << module_label
            << " is a modifier/observer whereas the other modules\n"
            << "on the path are the opposite.";
        }
      }
      if (present) {
        result.push_back(pr);
        paths_to_erase.push_back(path_name);
      }
    }
    for (auto const& path : paths_to_erase) {
      auto const path_it =
        std::find_if(paths_to_modules.cbegin(),
                     paths_to_modules.cend(),
                     [&path](auto const& pr) { return pr.first == path; });
      assert(path_it != paths_to_modules.cend());
      paths_to_modules.erase(path_it);
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

  std::set<art::ProductInfo>
  sorted_produced_products(
    std::vector<art::test::TypeAndInstance> const& productsToProduce,
    string const& module_name,
    string const& current_process_name)
  {
    std::set<art::ProductInfo> result;
    for (auto const& prod : productsToProduce) {
      result.emplace(
        art::ProductInfo::ConsumableType::Product,
        prod.friendlyClassName,
        module_name,
        prod.productInstanceName,
        art::ProcessTag{current_process_name, current_process_name});
    }
    return result;
  }

  template <typename T>
  auto
  consumables_for_module(Table<T> const& module,
                         string const& current_process_name)
  {
    std::vector<art::ProductInfo> sorted_deps;
    std::vector<art::test::TypeAndTag> deps;
    if (module().consumes(deps)) {
      for (auto const& dep : deps) {
        art::ProcessTag const processTag{dep.inputTag.process(),
                                         current_process_name};
        sorted_deps.emplace_back(art::ProductInfo::ConsumableType::Product,
                                 dep.friendlyClassName,
                                 dep.inputTag.label(),
                                 dep.inputTag.instance(),
                                 processTag);
      }
    }
    std::vector<std::string> many;
    if (module().consumesMany(many)) {
      for (auto const& class_name : many) {
        sorted_deps.emplace_back(art::ProductInfo::ConsumableType::Many,
                                 class_name);
      }
    }
    cet::sort_all(sorted_deps);
    art::ConsumesInfo::consumables_t::mapped_type result{{}};
    result[art::InEvent] = std::move(sorted_deps);
    return result;
  }

  void
  fillProducesInfo(
    ParameterSet const& pset,
    string const& process_name,
    string const& path_name,
    configs_t const& module_configs,
    std::map<std::string, std::set<art::ProductInfo>>& produced_products,
    collection_map_t& modules)
  {
    auto const begin = cbegin(module_configs);
    for (auto it = begin, end = cend(module_configs); it != end; ++it) {
      auto const& module_name =
        it->moduleConfigInfo->modDescription.moduleLabel();
      auto& info = modules[module_name];
      info.paths.insert(path_name);
      info.module_type = module_found_with_type(module_name, pset);
      auto const table_name = table_for_module_type(info.module_type);
      auto const& table =
        pset.get<ParameterSet>(table_name + "." + module_name);
      if (!is_modifier(info.module_type))
        continue;

      Table<art::test::ModifierModuleConfig> const mod{table};
      std::vector<art::test::TypeAndInstance> prods;
      if (mod().produces(prods)) {
        info.produced_products =
          sorted_produced_products(prods, module_name, process_name);
        produced_products[module_name] = info.produced_products;
      }
    }
  }

  void
  fillModifierInfo(
    ParameterSet const& pset,
    string const& process_name,
    string const& path_name,
    configs_t const& module_configs,
    std::map<std::string, std::set<art::ProductInfo>> const& produced_products,
    collection_map_t& modules)
  {
    auto const begin = cbegin(module_configs);
    for (auto it = begin, end = cend(module_configs); it != end; ++it) {
      auto const& module_name =
        it->moduleConfigInfo->modDescription.moduleLabel();
      auto& info = modules[module_name];
      info.paths.insert(path_name);
      info.module_type = module_found_with_type(module_name, pset);
      auto const table_name = table_for_module_type(info.module_type);
      auto const& table =
        pset.get<ParameterSet>(table_name + "." + module_name);
      if (!is_modifier(info.module_type))
        continue;

      Table<art::test::ModifierModuleConfig> const mod{table};
      auto const consumables = consumables_for_module(mod, process_name);
      info.consumed_products = consumed_products_for_module(
        process_name, consumables, produced_products, {}, begin, it);
    }
  }

  void
  fillObserverInfo(
    ParameterSet const& pset,
    string const& process_name,
    string const& path_name,
    configs_t const& module_configs,
    std::map<std::string, std::set<art::ProductInfo>> const& produced_products,
    collection_map_t& modules)
  {
    auto const begin = cbegin(module_configs);
    for (auto it = begin, end = cend(module_configs); it != end; ++it) {
      auto const& module_name =
        it->moduleConfigInfo->modDescription.moduleLabel();
      auto& info = modules[module_name];
      info.paths.insert(path_name);
      info.module_type = module_found_with_type(module_name, pset);
      auto const table_name = table_for_module_type(info.module_type);
      auto const& table =
        pset.get<ParameterSet>(table_name + "." + module_name);
      if (!is_observer(info.module_type))
        continue;

      Table<art::test::ObserverModuleConfig> const mod{table};
      auto const consumables = consumables_for_module(mod, process_name);
      info.consumed_products = consumed_products_for_module(
        process_name, consumables, produced_products, {}, begin, it);
      std::vector<string> sel;
      if (mod().select_events(sel)) {
        info.select_events = std::set<string>(cbegin(sel), cend(sel));
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
  Table<art::test::TopLevelTable> table{pset};
  auto const& process_name = table().process_name();
  auto const& test_properties = table().test_properties();

  ParameterSet physics;
  if (!table().physics.get_if_present(physics)) {
    return 0;
  }

  // Form the paths
  auto module_configs = get_module_configs(pset);
  auto paths_to_modules = get_paths_to_modules(physics, module_configs);
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

  // Assemble all the information for products to be produced.
  std::map<std::string, std::set<art::ProductInfo>> produced_products{};
  for (auto const& path : trigger_paths) {
    fillProducesInfo(
      pset, process_name, path.first, path.second, produced_products, modules);
  }

  // Now go through an assemble the rest of the graph info objects,
  // based on the consumes clauses.  The reason this is separate from
  // the filling of the produces information is that we want to allow
  // users to specify consumes dependencies at this stage, checking
  // for correct types, etc. *before* checking if the workflow is
  // well-formed (i.e. no interpath dependencies, or intrapath
  // circularities).  This pattern mimics what is done in
  // art::PathManager, where all produces information is filled first,
  // and then the graph is assembled afterward.
  std::string err_msg;
  bool graph_failure{false};
  try {
    for (auto const& path : trigger_paths) {
      fillModifierInfo(pset,
                       process_name,
                       path.first,
                       path.second,
                       produced_products,
                       modules);
    }

    if (!trigger_paths.empty()) {
      modules["TriggerResults"] = ModuleGraphInfo{art::ModuleType::producer};
    }

    fillObserverInfo(
      pset, process_name, "end_path", end_path, produced_products, modules);
  }
  catch (cet::exception const& e) {
    err_msg += e.what();
    graph_failure = true;
  }

  // Build the graph only if there was no error in constructing the
  // information it needs.
  if (err_msg.empty()) {
    ModuleGraphInfoMap const modInfos{modules};
    auto const module_graph =
      art::detail::make_module_graph(modInfos, trigger_paths, end_path);
    auto const& err = module_graph.second;
    if (!err.empty()) {
      err_msg += err;
      graph_failure = true;
    }

    auto const pos = filename.find(".fcl");
    string const basename =
      (pos != string::npos) ? filename.substr(0, pos) : filename;
    std::ofstream ofs{basename + ".dot"};
    print_module_graph(ofs, modInfos, module_graph.first);
  }

  // Check if test properties have been satisfied
  int rc{};
  bool const graph_failure_expected = test_properties.graph_failure_expected();
  if (graph_failure && !graph_failure_expected) {
    std::cerr << "Unexpected graph-construction failure.\n"
              << "Error message:\n"
              << err_msg << '\n';
    rc = 1;
  } else if (!graph_failure && graph_failure_expected) {
    std::cerr << "Unexpected graph-construction success.\n";
    rc = 1;
  }
  string expected_msg;
  if (test_properties.error_message(expected_msg)) {
    std::regex const re{expected_msg};
    if (!std::regex_search(err_msg, re)) {
      std::cerr << " The error message does not match what was expected:\n"
                << "   Actual: [" << err_msg << "]\n"
                << "   Expected: [" << expected_msg << "]\n";
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
