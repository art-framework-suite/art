#include "art/Framework/Art/detail/prune_configuration.h"
#include "art/Framework/Art/detail/exists_outside_prolog.h"
#include "art/Framework/Art/detail/fhicl_key.h"
#include "art/Persistency/Provenance/PathSpec.h"
#include "boost/algorithm/string.hpp"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/extended_value.h"
#include "range/v3/view.hpp"

#include <initializer_list>
#include <iostream>
#include <regex>
#include <set>

using namespace fhicl;
using namespace std::string_literals;
using namespace art::detail;

using sequence_t = fhicl::extended_value::sequence_t;
using table_t = fhicl::extended_value::table_t;
using modules_t = std::map<std::string, std::string>;

namespace {

  std::string const at_nil{"@nil"};
  std::string const trigger_paths_str{"trigger_paths"};
  std::string const end_paths_str{"end_paths"};
  bool
  matches(std::string const& path_spec_str, std::string const& path_name)
  {
    std::regex const path_spec_re{R"((\d+:)?)" + path_name};
    return std::regex_match(path_spec_str, path_spec_re);
  }

  auto module_tables = {"physics.producers",
                        "physics.filters",
                        "physics.analyzers",
                        "outputs"};
  auto modifier_tables = {"physics.producers", "physics.filters"};
  auto observer_tables = {"physics.analyzers", "outputs"};

  auto allowed_physics_tables = {"producers", "filters", "analyzers"};

  enum class ModuleCategory { modifier, observer, unset };

  auto
  config_exception(std::string const& context)
  {
    return art::Exception{art::errors::Configuration, context + "\n"};
  }

  auto
  path_exception(std::string const& selection_override,
                 std::size_t const i,
                 std::string const& suffix)
  {
    std::string msg{"The following error occurred while processing " +
                    selection_override + "[" + std::to_string(i) + "]"};
    msg += " (i.e. '" + suffix + "'):";
    return config_exception(msg);
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
    auto const table =
      full_module_key.substr(0, full_module_key.find_last_of('.'));
    if (cet::search_all(modifier_tables, table)) {
      return ModuleCategory::modifier;
    }
    if (cet::search_all(observer_tables, table)) {
      return ModuleCategory::observer;
    }
    return ModuleCategory::unset;
  }

  bool
  is_path_selection_override(std::string const& path_name)
  {
    return path_name == trigger_paths_str or path_name == end_paths_str;
  }

  std::string
  path_selection_override(ModuleCategory const category)
  {
    switch (category) {
    case ModuleCategory::modifier:
      return trigger_paths_str;
    case ModuleCategory::observer:
      return end_paths_str;
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

}

namespace art::detail {
  std::vector<ModuleSpec>
  sequence_to_entries(sequence_t const& seq, bool const allow_nil_entries)
  {
    std::vector<ModuleSpec> result;
    for (auto const& ev : seq) {
      if (allow_nil_entries and ev.is_a(NIL)) {
        result.push_back({at_nil, FilterAction::Normal});
        continue;
      }
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

  std::vector<art::PathSpec>
  path_specs(std::vector<ModuleSpec> const& selection_override_entries,
             std::string const& path_selection_override)
  {
    auto guidance = [](std::string const& name,
                       std::string const& path_selection_override,
                       std::string const& id_str) {
      std::ostringstream oss;
      oss << "If you would like to repeat the path specification, all "
             "path specifications\n"
             "with the name '"
          << name << "' must be prepended with the same path ID (e.g.):\n\n"
          << "  " << path_selection_override << ": ['" << id_str << ':' << name
          << "', '" << id_str << ':' << name << "', ...]\n\n";
      return oss.str();
    };

    std::map<PathID, std::string> id_to_name;
    std::map<std::string, PathID> name_to_id;
    std::vector<art::PathSpec> result;

    size_t i = 0;
    for (auto it = cbegin(selection_override_entries),
              e = cend(selection_override_entries);
         it != e;
         ++it, ++i) {
      auto const& path = *it;
      auto spec = art::path_spec(path.name);
      if (spec.name == at_nil) {
        continue;
      }

      // Path names with unspecified IDs cannot be reused
      auto const emplacement_result =
        name_to_id.try_emplace(spec.name, spec.path_id);
      bool const name_already_present = not emplacement_result.second;
      auto const emplaced_path_id = emplacement_result.first->second;

      if (name_already_present) {
        if (spec.path_id == art::PathID::invalid()) {
          throw path_exception(path_selection_override, i, path.name)
            << "The path name '" << spec.name
            << "' has already been specified in the " << path_selection_override
            << " sequence.\n"
            << guidance(spec.name,
                        path_selection_override,
                        to_string(emplaced_path_id));
        }
        if (spec.path_id != emplaced_path_id) {
          throw path_exception(path_selection_override, i, path.name)
            << "The path name '" << spec.name
            << "' has already been specified (perhaps implicitly) with a\n"
               "path ID of "
            << to_string(emplaced_path_id) << " (not "
            << to_string(spec.path_id) << ") in the " << path_selection_override
            << " sequence.\n\n"
            << guidance(spec.name,
                        path_selection_override,
                        to_string(emplaced_path_id));
        }
        // Name is already present and the PathID has been explicitly
        // listed and matches what has already been seen.
        continue;
      }

      if (spec.path_id == art::PathID::invalid()) {
        spec.path_id =
          art::PathID{i}; // Use calculated bit number if not specified
        emplacement_result.first->second = spec.path_id;
      }

      // Each ID must have only one name
      if (auto const [it, inserted] =
            id_to_name.try_emplace(spec.path_id, spec.name);
          not inserted) {
        throw path_exception(path_selection_override, i, path.name)
          << "Path ID " << to_string(spec.path_id)
          << " cannot be assigned to path name '" << spec.name
          << "' as it has already been assigned to path name '" << it->second
          << "'.\n";
      }

      result.push_back(std::move(spec));
    }
    return result;
  }
}

namespace {
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

  void
  replace_empty_paths(intermediate_table& config,
                      std::set<std::string> const& empty_paths,
                      std::string const& path_selection_override)
  {
    if (not exists_outside_prolog(config, path_selection_override)) {
      return;
    }
    sequence_t result = config.find(path_selection_override);
    for (auto const& name : empty_paths) {
      std::replace_if(begin(result),
                      end(result),
                      [&name](auto const& ex_val) {
                        if (not ex_val.is_a(STRING)) {
                          return false;
                        }
                        std::string path_spec_str;
                        fhicl::detail::decode(ex_val.value, path_spec_str);
                        return matches(path_spec_str, name);
                      },
                      fhicl::extended_value{false, NIL, at_nil});
    }
    config.get<sequence_t&>(path_selection_override) = result;
  }

  module_entries_for_path_t
  all_paths(intermediate_table& config)
  {
    std::string const physics{"physics"};
    if (!exists_outside_prolog(config, physics))
      return {};

    std::set<std::string> empty_paths;
    std::map<std::string, std::vector<ModuleSpec>> paths;
    table_t const& table = config.find(physics);
    verify_supported_names(table);
    for (auto const& [path_name, module_names] : table) {
      if (!module_names.is_a(SEQUENCE)) {
        continue;
      }
      sequence_t const entries = module_names;
      if (empty(entries)) {
        empty_paths.insert(path_name);
      }
      paths[path_name] = sequence_to_entries(
        module_names, is_path_selection_override(path_name));
    }

    // Replace empty paths from trigger_paths and end_paths with @nil
    replace_empty_paths(
      config, empty_paths, fhicl_key(physics, trigger_paths_str));
    replace_empty_paths(config, empty_paths, fhicl_key(physics, end_paths_str));

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
    for (auto const& [path_name, entries] : all_paths) {
      // Skip over special path names, which are handled later.
      if (is_path_selection_override(path_name)) {
        continue;
      }
      std::vector<ModuleSpec> right_modules;
      std::vector<std::string> wrong_modules;
      for (auto const& mod_spec : entries) {
        auto const& name = mod_spec.name;
        auto full_module_key_it = modules.find(name);
        if (full_module_key_it == cend(modules)) {
          throw config_exception("The following error occurred while "
                                 "processing a path configuration:")
            << "Entry with name " << name << " in path " << path_name
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
        sorted_result.try_emplace(path_name, move(right_modules));
      } else {
        // This is the case where a path contains a mixture of
        // modifiers and observers.
        auto e = config_exception("An error occurred while "
                                  "processing a path configuration.");
        e << "The following modules specified in path " << path_name << " are "
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
    module_entries_for_ordered_path_t result;
    size_t i = 0;
    for (auto const& [path_name, modules] : sorted_result) {
      result.emplace_back(art::PathSpec{path_name, art::PathID{i++}}, modules);
    }
    return result;
  }

  module_entries_for_ordered_path_t
  explicitly_declared_paths(module_entries_for_path_t const& modules_for_path,
                            std::vector<ModuleSpec> const& override_entries,
                            modules_t const& modules,
                            std::string const& path_selection_override)
  {
    auto specs =
      art::detail::path_specs(override_entries, path_selection_override);

    module_entries_for_ordered_path_t result;

    std::ostringstream os;
    for (auto& spec : specs) {
      auto res = modules_for_path.find(spec.name);
      if (res == cend(modules_for_path)) {
        os << "Unknown path " << spec.name << " has been specified in '"
           << path_selection_override << "'.\n";
        continue;
      }

      // Skip empty paths
      if (empty(res->second)) {
        continue;
      }

      // Check that module names in paths are supported
      for (auto const& entry : res->second) {
        auto const& name = entry.name;
        auto full_module_key_it = modules.find(name);
        if (full_module_key_it == cend(modules)) {
          throw config_exception("The following error occurred while "
                                 "processing a path configuration:")
            << "Entry with name " << name << " in path " << spec.name
            << " does not have a module configuration.\n";
        }
      }
      result.emplace_back(std::move(spec), res->second);
    }

    auto const err = os.str();
    if (!err.empty()) {
      throw config_exception(
        "The following error occurred while processing path configurations:")
        << err;
    }

    return result;
  }

  keytype_for_name_t
  get_enabled_modules(modules_t const& modules,
                      module_entries_for_ordered_path_t const& enabled_paths,
                      ModuleCategory const category)
  {
    keytype_for_name_t result;
    for (auto const& [path_spec, entries] : enabled_paths) {
      for (auto const& [module_name, action] : entries) {
        auto const& module_key = modules.at(module_name);
        auto const actual_category = module_category(module_key);
        auto const type = module_type(module_key);
        if (actual_category != category) {
          throw config_exception("The following error occurred while "
                                 "processing a path configuration:")
            << "The '" << path_selection_override(category)
            << "' override parameter contains the path " << path_spec.name
            << ", which has"
            << (actual_category == ModuleCategory::observer ? " an\n" : " a\n")
            << to_string(type) << " with the name " << module_name << ".\n\n"
            << "Path " << path_spec.name
            << " should instead be included as part of the '"
            << path_selection_override(opposite(category)) << "' parameter.\n"
            << "Contact artists@fnal.gov for guidance.\n";
        }
        if (action != art::detail::FilterAction::Normal &&
            type != art::ModuleType::filter) {
          throw config_exception("The following error occurred while "
                                 "processing a path configuration:")
            << "Entry with name " << module_name << " in path "
            << path_spec.name << " is"
            << (category == ModuleCategory::observer ? " an " : " a ")
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
    auto const selection_override = path_selection_override(category);
    auto const it = paths.find(selection_override);
    if (it == cend(paths)) {
      return {paths_for_category(paths, modules, category), false};
    }

    return {
      explicitly_declared_paths(paths, it->second, modules, selection_override),
      true};
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
  using namespace ranges;
  paths.erase("trigger_paths");
  paths.erase("end_paths");
  for (auto const& spec : trigger_paths | views::keys) {
    paths.erase(spec.name);
  }
  for (auto const& spec : end_paths | views::keys) {
    paths.erase(spec.name);
  }

  // The only paths left are those that are not enabled for execution.
  if (report_enabled && !empty(paths)) {
    std::cerr << "The following paths have not been enabled for execution and "
                 "will be ignored:\n";
    for (auto const& path_name : paths | views::keys) {
      std::cerr << "  " << path_name << '\n';
    }
  }

  if (report_enabled && !empty(unused_modules)) {
    std::ostringstream os;
    os << "The following module label"
       << ((unused_modules.size() == 1) ? " is" : "s are")
       << " either not assigned to any path,\n"
       << "or " << ((unused_modules.size() == 1ull) ? "it has" : "they have")
       << " been assigned to ignored path(s):\n";
    for (auto const& label : unused_modules | views::keys) {
      os << "  " << label << '\n';
    }
    std::cerr << os.str();
  }

  if (prune_config) {
    auto to_full_path_name = [](auto const& path_name) {
      return fhicl_key("physics", path_name);
    };
    for (auto const& key :
         paths | views::keys | views::transform(to_full_path_name)) {
      config.erase(key);
    }
    for (auto const& label : unused_modules | views::values) {
      config.erase(label);
    }

    // Check if module tables can be removed
    auto if_outside_prolog = [&config](auto const& table_name) {
      return exists_outside_prolog(config, table_name);
    };
    for (auto const& table_name :
         module_tables | views::filter(if_outside_prolog)) {
      if (table_t const value = config.find(table_name); empty(value)) {
        config.erase(table_name);
      }
    }

    // Check if top-level physics table can be removed
    if (exists_outside_prolog(config, "physics")) {
      if (table_t const value = config.find("physics"); empty(value)) {
        config.erase("physics");
      }
    }
  }

  // Ensure that trigger_paths/end_paths is present in the configuration
  if (not empty(end_paths) and not end_paths_override) {
    // We do not return the Path ID for end paths as they are not meaningful.
    auto end_paths_entries =
      end_paths | views::keys |
      views::transform([](auto const& path_spec) { return path_spec.name; }) |
      to<std::vector>();

    config.put("physics.end_paths", move(end_paths_entries));
  }

  // Place 'trigger_paths' as top-level configuration table
  if (not empty(trigger_paths)) {
    auto trigger_paths_entries = trigger_paths | views::keys |
                                 views::transform([](auto const& path_spec) {
                                   return to_string(path_spec);
                                 }) |
                                 to<std::vector>();
    if (not trigger_paths_override) {
      config.put("physics.trigger_paths", trigger_paths_entries);
    }
    config.put("trigger_paths.trigger_paths", move(trigger_paths_entries));
  }

  return EnabledModules{std::move(enabled_modules),
                        std::move(trigger_paths),
                        std::move(end_paths),
                        trigger_paths_override,
                        end_paths_override};
}
