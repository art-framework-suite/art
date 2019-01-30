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
using sequence_t = fhicl::extended_value::sequence_t;
using table_t = fhicl::extended_value::table_t;
using art::detail::exists_outside_prolog;
using art::detail::fhicl_key;
using art::detail::modules_per_path_t;
using art::detail::modules_t;

namespace {

  auto module_tables = {"physics.producers",
                        "physics.filters",
                        "physics.analyzers",
                        "outputs"};
  auto modifier_tables = {"physics.producers", "physics.filters"};
  auto observer_tables = {"physics.analyzers", "outputs"};

  enum class ModuleCategory { modifier, observer, unset };

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
    } else {
      return ModuleCategory::unset;
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
      for (auto const& pr : table) {
        // Record only tables, which are the allowed FHiCL values for
        // module configurations.
        auto const& modname = pr.first;
        if (pr.second.is_a(TABLE)) {
          result.emplace(modname, fhicl_key(tbl, modname));
        }
      }
    }
    return result;
  }

  auto
  sequence_to_strings(sequence_t const& seq)
  {
    std::vector<std::string> result;
    for (auto const& ev : seq) {
      if (!ev.is_a(STRING)) {
        continue;
      }
      auto mod_spec = ev.to_string();
      boost::replace_all(mod_spec, "\"", "");
      boost::replace_all(mod_spec, "!", "");
      boost::replace_all(mod_spec, "-", "");
      result.push_back(mod_spec);
    }
    if (result.size() != seq.size()) {
      throw art::Exception{art::errors::Configuration,
                           "There was an error parsing the specified entries "
                           "in a FHiCL sequence.\n"}
        << "One of the presented elements is not a string.\n";
    }
    return result;
  }

  std::map<std::string, std::vector<std::string>>
  all_paths(intermediate_table const& config)
  {
    std::string const physics{"physics"};
    if (!exists_outside_prolog(config, physics))
      return {};

    std::map<std::string, std::vector<std::string>> paths;
    table_t const& table = config.find(physics);
    for (auto const& pr : table) {
      auto const& pathname = pr.first;
      auto const& modules = pr.second;
      if (!modules.is_a(SEQUENCE)) {
        continue;
      }
      paths[pathname] = sequence_to_strings(modules);
    }
    return paths;
  }

  modules_per_path_t
  paths_for_tables(modules_per_path_t& all_paths,
                   modules_t const& modules,
                   ModuleCategory const category)
  {
    using table_t = fhicl::extended_value::table_t;
    modules_per_path_t result;
    std::vector<std::string> paths_to_remove;
    for (auto const& pr : all_paths) {
      auto const& pathname = pr.first;
      // Skip over special path names, which are handled later.
      if (pathname == "trigger_paths" || pathname == "end_paths") {
        continue;
      }
      std::vector<std::string> right_modules;
      std::vector<std::string> wrong_modules;
      for (auto const& modname : pr.second) {
        auto full_module_key_it = modules.find(modname);
        if (full_module_key_it == cend(modules)) {
          throw art::Exception{art::errors::Configuration,
                               "The following error was encountered while "
                               "processing a path configuration:\n"}
            << "Entry " << modname << " in path " << pathname
            << " does not have a module configuration.\n";
        }
        auto const& full_module_key = full_module_key_it->second;
        auto const module_cat = module_category(full_module_key);
        assert(module_cat != ModuleCategory::unset);
        if (module_cat == category) {
          right_modules.push_back(modname);
        } else {
          wrong_modules.push_back(modname);
        }
      }

      if (right_modules.empty()) {
        // None of the modules in the path was of the correct
        // category.  This means that all modules were of the opposite
        // category--we can safely skip it.
        continue;
      }

      if (right_modules.size() == pr.second.size()) {
        result.emplace(pathname, move(right_modules));
        // Cannot immediately erase since range-for will attempt to
        // increment an invalid iterator corresponding to pathname.
        paths_to_remove.push_back(pathname);
      } else {
        // This is the case where a path contains a mixture of
        // modifiers and observers.
        art::Exception e{art::errors::Configuration,
                         "An error was encountered while "
                         "processing a path configuration.\n"};
        e << "The following entries in path " << pathname << " are "
          << opposite(category)
          << "s when all other\n"
             "entries are "
          << category << "s:\n";
        for (auto const& modname : wrong_modules) {
          e << "  '" << modname << "'\n";
        }
        throw e;
      }
    }
    for (auto const& path : paths_to_remove) {
      all_paths.erase(path);
    }
    return result;
  }

  std::optional<modules_per_path_t>
  explicitly_declared_paths(modules_per_path_t& modules_per_path,
                            std::string const& controlling_sequence)
  {
    auto const it = modules_per_path.find(controlling_sequence);
    if (it == cend(modules_per_path)) {
      return std::nullopt;
    }

    std::ostringstream os;
    modules_per_path_t result;
    std::set<std::string> paths_to_erase;
    for (auto const& pathname : it->second) {
      auto res = modules_per_path.find(pathname);
      if (res == cend(modules_per_path)) {
        os << "ERROR: Unknown path " << pathname << " specified by user in "
           << controlling_sequence << ".\n";
        continue;
      }
      result.insert(*res);
      paths_to_erase.insert(pathname);
    }

    for (auto const& path_to_erase : paths_to_erase) {
      modules_per_path.erase(path_to_erase);
    }
    modules_per_path.erase(it); // Remove controlling sequence

    auto const err = os.str();
    if (!err.empty()) {
      throw art::Exception{art::errors::Configuration} << err;
    }
    return std::make_optional<modules_per_path_t>(std::move(result));
  }
}

std::pair<modules_per_path_t, modules_t>
art::detail::detect_unused_configuration(intermediate_table& config)
{
  auto const modules = declared_modules(config, module_tables);

  auto paths = all_paths(config);

  auto trigger_paths = explicitly_declared_paths(paths, "trigger_paths");
  auto enabled_trigger_paths =
    trigger_paths ? *trigger_paths :
                    paths_for_tables(paths, modules, ModuleCategory::modifier);

  auto end_paths = explicitly_declared_paths(paths, "end_paths");
  auto enabled_end_paths =
    end_paths ? *end_paths :
                paths_for_tables(paths, modules, ModuleCategory::observer);

  // The only paths left are those that are not enabled for execution.
  if (!paths.empty()) {
    std::cerr << "The following paths have not been enabled for execution and "
                 "will be ignored:\n";
    for (auto const& pr : paths) {
      std::cerr << "  " << pr.first << '\n';
    }
  }

  std::set<std::string> enabled_modules;
  for (auto const& pr : enabled_trigger_paths) {
    enabled_modules.insert(cbegin(pr.second), cend(pr.second));
  }
  for (auto const& pr : enabled_end_paths) {
    enabled_modules.insert(cbegin(pr.second), cend(pr.second));
  }

  std::map<std::string, std::string> unused_modules;
  cet::copy_if_all(modules,
                   inserter(unused_modules, end(unused_modules)),
                   [& emods = enabled_modules](auto const& mod) {
                     return emods.find(mod.first) == cend(emods);
                   });

  if (!unused_modules.empty()) {
    std::ostringstream os;
    auto i = cbegin(unused_modules);
    os << "The following module label"
       << ((unused_modules.size() == 1) ? " is" : "s are")
       << " either not assigned to any path,\n"
       << "or " << ((unused_modules.size() == 1ull) ? "it has" : "they have")
       << " been assigned to ignored path(s):\n"
       << "'" << i->first << "'";
    ++i;
    for (auto e = cend(unused_modules); i != e; ++i) {
      os << ", '" << i->first << "'";
    }
    std::cerr << os.str() << '\n';
  }
  return std::make_pair(move(paths), move(unused_modules));
}

void
art::detail::prune_configuration(modules_per_path_t const& unused_paths,
                                 modules_t const& unused_modules,
                                 intermediate_table& config)
{
  for (auto const& pr : unused_paths) {
    config.erase(fhicl_key("physics", pr.first));
  }
  for (auto const& pr : unused_modules) {
    config.erase(pr.second);
  }
}
