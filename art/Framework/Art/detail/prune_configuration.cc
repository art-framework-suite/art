#include "art/Framework/Art/detail/prune_configuration.h"
#include "art/Framework/Art/detail/exists_outside_prolog.h"
#include "art/Framework/Art/detail/fhicl_key.h"
#include "boost/algorithm/string.hpp"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/extended_value.h"

#include <initializer_list>
#include <iostream>
#include <optional>
#include <set>

using namespace fhicl;
using namespace std::string_literals;
using sequence_t = fhicl::extended_value::sequence_t;
using table_t = fhicl::extended_value::table_t;
using namespace art::detail;

using modules_t = std::map<std::string, std::string>;

namespace {

  auto module_tables = {"physics.producers",
                        "physics.filters",
                        "physics.analyzers",
                        "outputs"};
  auto modifier_tables = {"physics.producers", "physics.filters"};
  auto observer_tables = {"physics.analyzers", "outputs"};

  auto allowed_physics_tables = {"producers", "filters", "analyzers"};

  enum class ModuleCategory { modifier, observer, unset };

  art::Exception
  config_exception(std::string const& context)
  {
    return art::Exception{art::errors::Configuration, context + "\n"};
  }

  std::ostream&
  operator<<(std::ostream& os, ModuleCategory const cat)
  {
    switch (cat) {
      case ModuleCategory::modifier:
        os << "modifier";
        break;
      case ModuleCategory::observer:
        os << "observer";
        break;
      case ModuleCategory::unset:
        os << "unset";
    }
    return os;
  }

  ModuleCategory
  opposite(ModuleCategory const cat)
  {
    auto result = ModuleCategory::unset;
    switch (cat) {
      case ModuleCategory::modifier:
        result = ModuleCategory::observer;
        break;
      case ModuleCategory::observer:
        result = ModuleCategory::modifier;
        break;
      case ModuleCategory::unset: {
        throw art::Exception{art::errors::LogicError}
          << "The " << cat << " category has no opposite.\n";
      }
    }
    return result;
  }

  auto
  module_category(std::string const& full_module_key)
  {
    // For a full module key (e.g. physics.analyzers.a), we strip off
    // the module name ('a') to determine its module category.
    auto table = full_module_key.substr(0, full_module_key.find_last_of('.'));
    if (cet::search_all(modifier_tables, table)) {
      return ModuleCategory::modifier;
    } else if (cet::search_all(observer_tables, table)) {
      return ModuleCategory::observer;
    }
    return ModuleCategory::unset;
  }

  std::string
  path_selection_override(ModuleCategory const category)
  {
    switch (category) {
      case ModuleCategory::modifier:
        return "trigger_paths";
      case ModuleCategory::observer:
        return "end_paths";
      default:
        assert(false); // Unreachable
        return {};
    }
  }

  // module name => full module key
  modules_t
  declared_modules(intermediate_table const& config,
                   std::initializer_list<char const*> tables)
  {
    modules_t result;
    for (auto const tbl : tables) {
      if (!exists_outside_prolog(config, tbl))
        continue;
      table_t const& table = config.find(tbl);
      for (auto const& [modname, value] : table) {
        // Record only tables, which are the allowed FHiCL values for
        // module configurations.
        if (!value.is_a(TABLE)) {
          continue;
        }

        auto const key = fhicl_key(tbl, modname);
        auto const [it, success] = result.try_emplace(modname, fhicl_key(key));
        if (!success && it->second != key) {
          auto const& cached_key = it->second;
          auto const parent =
            cached_key.substr(0, cached_key.rfind(modname) - 1);
          throw config_exception("An error occurred while processing "
                                 "module configurations.")
            << "Module label '" << modname << "' has been used in '" << tbl
            << "' and '" << parent << "'.\n"
            << "Module labels must be unique across an art process.\n";
        }
      }
    }
    return result;
  }

  auto
  sequence_to_entries(sequence_t const& seq)
  {
    std::vector<PathEntry> result;
    for (auto const& ev : seq) {
      if (!ev.is_a(STRING)) {
        continue;
      }
      auto mod_spec = ev.to_string();
      if (empty(mod_spec)) {
        continue;
      }
      boost::replace_all(mod_spec, "\"", "");
      auto action = FilterAction::Normal;
      if (mod_spec[0] == '!') {
        action = FilterAction::Veto;
        mod_spec = mod_spec.substr(1);
      } else if (mod_spec[0] == '-') {
        action = FilterAction::Ignore;
        mod_spec = mod_spec.substr(1);
      }

      // Handle remaining '!' or '-' characters
      if (mod_spec.find_first_of("!-") != std::string::npos) {
        throw config_exception("There was an error parsing the entry "s +
                               ev.to_string() + "in a FHiCL sequence.")
          << "The '!' or '-' character may appear as only the first character "
             "in the path entry.\n";
      }
      result.push_back({mod_spec, action});
    }
    if (result.size() != seq.size()) {
      throw config_exception("There was an error parsing the specified entries "
                             "in a FHiCL sequence.")
        << "One of the presented elements is either an empty string or not a "
           "string at all.\n";
    }
    return result;
  }

  void
  verify_supported_names(table_t const& physics_table)
  {
    std::string bad_names{};
    for (auto const& [name, value] : physics_table) {
      if (value.is_a(SEQUENCE)) {
        continue;
      }

      bool const is_table = value.is_a(TABLE);
      if (is_table and cet::search_all(allowed_physics_tables, name)) {
        continue;
      }
      std::string const type = is_table ? "table" : "atom";
      bad_names += "   \"physics." + name + "\"   (" + type + ")\n";
    }

    if (empty(bad_names)) {
      return;
    }

    throw config_exception(
      "\nYou have specified the following unsupported parameters in the\n"
      "\"physics\" block of your configuration:\n")
      << bad_names
      << "\nSupported parameters include the following tables:\n"
         "   \"physics.producers\"\n"
         "   \"physics.filters\"\n"
         "   \"physics.analyzers\"\n"
         "and sequences. Atomic configuration parameters are not "
         "allowed.\n\n";
  }

  module_entries_for_path_t
  all_paths(intermediate_table const& config)
  {
    std::string const physics{"physics"};
    if (!exists_outside_prolog(config, physics))
      return {};

    std::map<std::string, std::vector<PathEntry>> paths;
    table_t const& table = config.find(physics);
    verify_supported_names(table);
    for (auto const& [path_name, module_names] : table) {
      if (!module_names.is_a(SEQUENCE)) {
        continue;
      }
      paths[path_name] = sequence_to_entries(module_names);
    }

    return paths;
  }

  // The return type of 'paths_for_category' is a vector of pairs -
  // i.e. hence the "ordered" component of the return type.  By
  // choosing this return type, we are able to preserve the
  // user-specified order in 'trigger_paths'.  In this case, art
  // specified the ordering for the user, but we use the return type
  // that matches that of the 'explicitly_declared_paths' function.

  module_entries_for_ordered_path_t
  paths_for_category(module_entries_for_path_t const& all_paths,
                     modules_t const& modules,
                     ModuleCategory const category)
  {
    using table_t = fhicl::extended_value::table_t;
    module_entries_for_path_t sorted_result;
    for (auto const& [pathname, entries] : all_paths) {
      // Skip over special path names, which are handled later.
      if (pathname == "trigger_paths" || pathname == "end_paths") {
        continue;
      }
      std::vector<PathEntry> right_modules;
      std::vector<std::string> wrong_modules;
      for (auto const& mod_spec : entries) {
        auto const& name = mod_spec.name;
        auto full_module_key_it = modules.find(name);
        if (full_module_key_it == cend(modules)) {
          throw config_exception("The following error occurred while "
                                 "processing a path configuration:")
            << "Entry with name " << name << " in path " << pathname
            << " does not have a module configuration.\n";
        }
        auto const& full_module_key = full_module_key_it->second;
        auto const module_cat = module_category(full_module_key);
        assert(module_cat != ModuleCategory::unset);
        if (module_cat == category) {
          right_modules.push_back(mod_spec);
        } else {
          wrong_modules.push_back(name);
        }
      }

      if (right_modules.empty()) {
        // None of the modules in the path was of the correct
        // category.  This means that all modules were of the opposite
        // category--we can safely skip it.
        continue;
      }

      if (right_modules.size() == entries.size()) {
        sorted_result.emplace(pathname, move(right_modules));
      } else {
        // This is the case where a path contains a mixture of
        // modifiers and observers.
        auto e = config_exception("An error occurred while "
                                  "processing a path configuration.");
        e << "The following modules specified in path " << pathname << " are "
          << opposite(category)
          << "s when all\n"
             "other modules are "
          << category << "s:\n";
        for (auto const& modname : wrong_modules) {
          e << "  '" << modname << "'\n";
        }
        throw e;
      }
    }
    // Convert to correct type
    return module_entries_for_ordered_path_t(sorted_result.begin(),
                                             sorted_result.end());
  }

  std::optional<module_entries_for_ordered_path_t>
  explicitly_declared_paths(module_entries_for_path_t const& modules_for_path,
                            modules_t const& modules,
                            std::string const& path_selection_override)
  {
    auto const it = modules_for_path.find(path_selection_override);
    if (it == cend(modules_for_path)) {
      return std::nullopt;
    }

    std::ostringstream os;
    module_entries_for_ordered_path_t result;
    for (auto const& path : it->second) {
      auto res = modules_for_path.find(path.name);
      if (res == cend(modules_for_path)) {
        os << "Unknown path " << path.name << " has been specified in '"
           << path_selection_override << "'.\n";
        continue;
      }

      // Check that module names in paths are supported
      for (auto const& entry : res->second) {
        auto const& name = entry.name;
        auto full_module_key_it = modules.find(name);
        if (full_module_key_it == cend(modules)) {
          throw config_exception("The following error occurred while "
                                 "processing a path configuration:")
            << "Entry with name " << name << " in path " << path.name
            << " does not have a module configuration.\n";
        }
      }
      result.push_back(*res);
    }

    auto const err = os.str();
    if (!err.empty()) {
      throw config_exception(
        "The following error occurred while processing path configurations:")
        << err;
    }
    return std::make_optional(std::move(result));
  }

  keytype_for_name_t
  get_enabled_modules(modules_t const& modules,
                      module_entries_for_ordered_path_t const& enabled_paths,
                      ModuleCategory const category)
  {
    keytype_for_name_t result;
    for (auto const& [path_name, entries] : enabled_paths) {
      for (auto const& [module_name, action] : entries) {
        auto const& module_key = modules.at(module_name);
        auto const actual_category = module_category(module_key);
        auto const type = module_type(module_key);
        if (actual_category != category) {
          throw config_exception("The following error occurred while "
                                 "processing a path configuration:")
            << "The '" << path_selection_override(category)
            << "' override parameter contains the path " << path_name
            << ", which has"
            << (actual_category == ModuleCategory::observer ? " an\n" : " a\n")
            << to_string(type) << " with the name " << module_name << ".\n\n"
            << "Path " << path_name
            << " should instead be included as part of the '"
            << path_selection_override(opposite(category)) << "' parameter.\n"
            << "Contact artists@fnal.gov for guidance.\n";
        }
        if (action != art::detail::FilterAction::Normal &&
            type != art::ModuleType::filter) {
          throw config_exception("The following error occurred while "
                                 "processing a path configuration:")
            << "Entry with name " << module_name << " in path " << path_name
            << " is" << (category == ModuleCategory::observer ? " an " : " a ")
            << to_string(type) << " and cannot have a '!' or '-' prefix.\n";
        }
        result.try_emplace(module_name, ModuleKeyAndType{module_key, type});
      }
    }
    return result;
  }

  std::pair<module_entries_for_ordered_path_t, bool>
  enabled_paths(module_entries_for_path_t const& paths,
                modules_t const& modules,
                ModuleCategory const category)
  {
    auto declared_paths = explicitly_declared_paths(
      paths, modules, path_selection_override(category));
    if (declared_paths) {
      return {std::move(*declared_paths), true};
    }
    return {paths_for_category(paths, modules, category), false};
  }
}

art::detail::EnabledModules
art::detail::prune_config_if_enabled(bool const prune_config,
                                     bool const report_enabled,
                                     intermediate_table& config)
{
  auto const modules = declared_modules(config, module_tables);

  auto paths = all_paths(config);

  auto [trigger_paths, trigger_paths_override] =
    enabled_paths(paths, modules, ModuleCategory::modifier);
  auto [end_paths, end_paths_override] =
    enabled_paths(paths, modules, ModuleCategory::observer);

  auto enabled_modules =
    get_enabled_modules(modules, trigger_paths, ModuleCategory::modifier);
  // C++17 provides the std::map::merge member function, but Clang 7
  // and older does not support it.  Will do it by hand for now, until
  // we have time to handle this properly.
  auto end_path_enabled_modules =
    get_enabled_modules(modules, end_paths, ModuleCategory::observer);
  enabled_modules.insert(begin(end_path_enabled_modules),
                         end(end_path_enabled_modules));

  modules_t unused_modules;
  for (auto const& pr : modules) {
    if (enabled_modules.find(pr.first) == cend(enabled_modules)) {
      unused_modules.insert(pr);
    }
  }

  // Find unused paths
  paths.erase("trigger_paths");
  paths.erase("end_paths");
  for (auto const& pr : trigger_paths) {
    paths.erase(pr.first);
  }
  for (auto const& pr : end_paths) {
    paths.erase(pr.first);
  }

  // The only paths left are those that are not enabled for execution.
  if (report_enabled && !empty(paths)) {
    std::cerr << "The following paths have not been enabled for execution and "
                 "will be ignored:\n";
    for (auto const& pr : paths) {
      std::cerr << "  " << pr.first << '\n';
    }
  }

  if (report_enabled && !empty(unused_modules)) {
    std::ostringstream os;
    os << "The following module label"
       << ((unused_modules.size() == 1) ? " is" : "s are")
       << " either not assigned to any path,\n"
       << "or " << ((unused_modules.size() == 1ull) ? "it has" : "they have")
       << " been assigned to ignored path(s):\n";
    for (auto const& pr : unused_modules) {
      os << "  " << pr.first << '\n';
    }
    std::cerr << os.str();
  }

  if (prune_config) {
    for (auto const& pr : paths) {
      config.erase(fhicl_key("physics", pr.first));
    }
    for (auto const& pr : unused_modules) {
      config.erase(pr.second);
    }
  }

  return EnabledModules{std::move(enabled_modules),
                        std::move(trigger_paths),
                        std::move(end_paths),
                        trigger_paths_override,
                        end_paths_override};
}
