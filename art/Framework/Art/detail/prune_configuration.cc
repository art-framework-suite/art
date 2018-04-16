#include "art/Framework/Art/detail/exists_outside_prolog.h"
#include "art/Framework/Art/detail/fhicl_key.h"
#include "art/Framework/Art/detail/prune_configuration.h"
#include "boost/algorithm/string.hpp"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/extended_value.h"

#include <initializer_list>
#include <iostream>
#include <set>

using namespace fhicl;
using sequence_t = fhicl::extended_value::sequence_t;
using table_t = fhicl::extended_value::table_t;
using art::detail::exists_outside_prolog;
using art::detail::fhicl_key;
using art::detail::modules_per_path_t;
using art::detail::modules_t;

namespace {

  // module name => full module key
  modules_t
  declared_modules(intermediate_table const& config,
                   std::initializer_list<char const*> tables)
  {
    modules_t result;
    for (auto const tbl : tables) {
      if (!exists_outside_prolog(config, tbl)) continue;
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
          "There was an error parsing the specified entries in a FHiCL sequence.\n"}
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
                   intermediate_table const& config,
                   std::initializer_list<char const*> tables)
  {
    using table_t = fhicl::extended_value::table_t;
    modules_per_path_t result;
    std::vector<std::string> paths_to_remove;
    for (auto const& pr : all_paths) {
      auto const& pathname = pr.first;
      std::vector<std::string> mods;
      for (auto const& modname : pr.second) {
        for (auto const table : tables) {
          auto const key = art::detail::fhicl_key(table, modname);
          if (art::detail::exists_outside_prolog(config, key)) {
            mods.push_back(modname);
            break;
          }
        }
      }
      if (mods.size() == pr.second.size()) {
        result.emplace(pathname, move(mods));
        // Cannot immediately erase since range-for will attempt to
        // increment an invalid iterator corresponding to pathname.
        paths_to_remove.push_back(pathname);
      }
    }
    for (auto const& path : paths_to_remove) {
      all_paths.erase(path);
    }
    return result;
  }

  std::unique_ptr<modules_per_path_t>
  explicitly_declared_paths(modules_per_path_t& modules_per_path,
                            std::string const& controlling_sequence)
  {
    auto const it = modules_per_path.find(controlling_sequence);
    if (it == cend(modules_per_path)) {
      return std::unique_ptr<modules_per_path_t>{nullptr};
    }

    std::ostringstream os;
    auto result = std::make_unique<modules_per_path_t>();
    for (auto const& pathname : it->second) {
      auto res = modules_per_path.find(pathname);
      if (res == cend(modules_per_path)) {
        os << "ERROR: Unknown path " << pathname
           << " specified by user in " << controlling_sequence << ".\n";
        continue;
      }
      result->insert(*res);
      modules_per_path.erase(res);
    }
    modules_per_path.erase(it); // Remove controlling sequence

    auto const err = os.str();
    if (!err.empty()) {
      throw art::Exception{art::errors::Configuration}
      << err;
    }
    return result;
  }

}

std::pair<modules_per_path_t, modules_t>
art::detail::detect_unused_configuration(intermediate_table& config)
{
  auto module_tables = {"physics.producers", "physics.filters", "physics.analyzers", "outputs"};
  auto const modules = declared_modules(config, module_tables);

  auto paths = all_paths(config);

  auto trigger_paths = explicitly_declared_paths(paths, "trigger_paths");
  auto modifier_tables = {"physics.producers", "physics.filters"};
  auto enabled_trigger_paths = trigger_paths ? *trigger_paths : paths_for_tables(paths, config, modifier_tables);

  auto end_paths = explicitly_declared_paths(paths, "end_paths");
  auto observer_tables = {"physics.analyzers", "outputs"};
  auto enabled_end_paths = end_paths ? *end_paths : paths_for_tables(paths, config, observer_tables);

  // The only paths left are those that are not enabled for execution.
  if (!paths.empty()) {
    std::cerr << "The following paths have not been enabled for execution and will be ignored:\n";
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
  std::copy_if(cbegin(modules), cend(modules),
               inserter(unused_modules, end(unused_modules)),
               [&emods=enabled_modules](auto const& mod) {
                 return emods.find(mod.first) == cend(emods);
               });

  if (!unused_modules.empty()) {
    std::ostringstream os;
    auto i = cbegin(unused_modules);
    os << "The following module label"
       << ((unused_modules.size() == 1) ? " is" : "s are")
       << " either not assigned to any path,\n"
       << "or "
       << ((unused_modules.size() == 1ull) ? "it has" : "they have")
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
