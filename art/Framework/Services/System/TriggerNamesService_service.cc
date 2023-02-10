#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Persistency/Provenance/PathSpec.h"
#include "art/Utilities/Globals.h"
#include "canvas/Persistency/Common/TriggerResults.h"
#include "canvas/Persistency/Provenance/ProcessConfiguration.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"

// vim: set sw=2 expandtab :

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <optional>
#include <string>
#include <vector>

using art::detail::entry_selector_t;
using namespace fhicl;
using namespace std;

using DataPerProcess = art::TriggerNamesService::DataPerProcess;

namespace {
  constexpr auto invalid_entry = numeric_limits<size_t>::max();
  entry_selector_t
  for_(art::PathID const id)
  {
    return [id](art::PathSpec const& spec) { return spec.path_id == id; };
  }
  entry_selector_t
  for_(string const& name)
  {
    return [&name](art::PathSpec const& spec) { return spec.name == name; };
  }

  auto
  lookup_exception(string const& process_name)
  {
    return art::Exception{
      art::errors::OtherArt,
      "An error occurred while retrieving the trigger paths for process '" +
        process_name + "'.\n"};
  }

  DataPerProcess
  data_for_process(ParameterSet const& trigger_paths_pset,
                   ParameterSet const physics_pset)
  {
    auto const spec_strs =
      trigger_paths_pset.get<vector<string>>("trigger_paths");
    auto specs = art::path_specs(spec_strs);

    vector<string> trigger_path_names;
    vector<vector<string>> module_names;
    for (auto const& spec_str : specs) {
      trigger_path_names.push_back(spec_str.name);
      module_names.push_back(physics_pset.get<vector<string>>(spec_str.name));
    }
    return {std::move(specs), std::move(trigger_path_names), std::move(module_names)};
  }
}

namespace art {

  TriggerNamesService::TriggerNamesService(
    ParameterSet const& trigger_paths_pset,
    ParameterSet const& physics_pset)
  {
    dataPerProcess_.try_emplace(
      getProcessName(), data_for_process(trigger_paths_pset, physics_pset));
  }

  DataPerProcess const&
  TriggerNamesService::currentData_() const
  {
    return dataPerProcess_.at(getProcessName());
  }

  // =================================================================================
  // All processes
  TriggerResults const&
  TriggerNamesService::triggerResults(Event const& e,
                                      string const& process_name) const
  {
    auto h = e.getHandle<TriggerResults>({"TriggerResults", "", process_name});
    if (h) {
      return *h;
    }
    throw lookup_exception(process_name) << *h.whyFailed();
  }

  map<string, HLTPathStatus>
  TriggerNamesService::pathResults(Event const& e,
                                   string const& process_name) const
  {
    auto const& tr = triggerResults(e, process_name);
    auto const& pname =
      process_name == "current_process" ? getProcessName() : process_name;

    auto it = dataPerProcess_.find(process_name);
    if (it == cend(dataPerProcess_)) {
      auto config = e.processHistory().getConfigurationForProcess(pname);
      if (not config) {
        throw lookup_exception(pname)
          << "Could not locate process configuration for the process '" << pname
          << "'\n"
          << "This can happen if the ParameterSets were dropped on input.\n"
          << "Please contact artists@fnal.gov for guidance.\n";
      }

      auto const& trigger_pset = ParameterSetRegistry::get(tr.parameterSetID());
      auto const& pset = ParameterSetRegistry::get(config->parameterSetID());
      auto data =
        data_for_process(trigger_pset, pset.get<ParameterSet>("physics"));
      it = dataPerProcess_.try_emplace(process_name, std::move(data)).first;
    }

    auto const& names = it->second.triggerPathNames;
    assert(size(names) == tr.size());

    map<string, HLTPathStatus> result;
    for (size_t i = 0, n = tr.size(); i != n; ++i) {
      result.try_emplace(names[i], tr.at(i));
    }
    return result;
  }

  // =================================================================================
  // Current process only
  string const&
  TriggerNamesService::getProcessName() const
  {
    return Globals::instance()->processName();
  }

  vector<string> const&
  TriggerNamesService::getTrigPaths() const
  {
    return currentData_().triggerPathNames;
  }

  string const&
  TriggerNamesService::getTrigPath(PathID const id) const
  {
    auto const i = index_for(id);
    if (i == invalid_entry) {
      throw Exception{errors::OtherArt}
        << "A path name could not be found corresponding to path ID "
        << to_string(id) << '\n';
    }
    return currentData_().triggerPathSpecs[i].name;
  }

  vector<string> const&
  TriggerNamesService::getTrigPathModules(string const& name) const
  {
    auto const& modules = currentData_().moduleNames;
    return modules.at(index_(for_(name)));
  }

  vector<string> const&
  TriggerNamesService::getTrigPathModules(PathID const id) const
  {
    auto const& modules = currentData_().moduleNames;
    return modules.at(index_for(id));
  }

  size_t
  TriggerNamesService::index_for(PathID const id) const
  {
    return index_(for_(id));
  }

  size_t
  TriggerNamesService::index_(entry_selector_t matched_entry) const
  {
    auto const& path_specs = currentData_().triggerPathSpecs;

    auto const b = begin(path_specs);
    auto const e = end(path_specs);
    auto it = find_if(b, e, matched_entry);
    if (it == e) {
      return invalid_entry;
    }
    return distance(b, it);
  }

} // namespace art
