#include "art/Framework/Core/PathManager.h"
#include "art/Utilities/Exception.h"

using fhicl::ParameterSet;

#include <algorithm>
#include <map>
#include <set>

art::PathManager::
PathManager(ParameterSet const & procPS,
            MasterProductRegistry & preg,
            ActionTable & exceptActions,
            std::shared_ptr<ActivityRegistry> const & areg)
  :
  procPS_(procPS),
  preg_(preg),
  exceptActions_(exceptActions),
  areg_(areg),
  fact_(),
  allModules_(fillAllModules_()),
  endPathWorkers_(),
  endPath_(),
  triggerPathWorkers_(),
  triggerPaths_(),
  triggerPathNames_(processPathConfigs_())
{
}

art::detail::ModuleConfigInfoMap
art::PathManager::
fillAllModules_()
{
  static ParameterSet const empty;
  art::detail::ModuleConfigInfoMap all_modules;
  for (auto const & pathRootName :
       art::detail::ModuleConfigInfo::allModulePathRoots()) {
    auto const pathRoot = procPS_.get<ParameterSet>(pathRootName, empty);
    auto const names = pathRoot.get_keys();
    for (auto const & name : names) {
      art::detail::ModuleConfigInfo mci(procPS_, name, pathRootName);
      auto actualModType = fact_.moduleType(mci.libSpec());
      if (actualModType != mci.moduleType()) {
        throw Exception(errors::Configuration)
            << "Module with label "
            << mci.simpleLabel()
            << " of type "
            << mci.libSpec()
            << " is configured as a "
            << to_string(mci.moduleType())
            << " but defined in code as a "
            << to_string(actualModType)
            << ".\n";
      }
      auto result =
        all_modules.insert(typename decltype(all_modules)::
                           value_type(mci.simpleLabel(), std::move(mci)));
      if (!result.second) {
        throw art::Exception(art::errors::Configuration)
            << "Module label "
            << mci.simpleLabel()
            << " has been used in "
            << result.first->second.configPath()
            << " and "
            << pathRootName
            << ".\n";
      }
    }
  }
  return std::move(all_modules);
}

art::PathManager::vstring
art::PathManager::
processPathConfigs_()
{
  vstring trigger_path_names;
  auto services = procPS_.get<ParameterSet>("services",
                                            ParameterSet());
  auto opts(services.get<ParameterSet>("scheduler", ParameterSet()));
  bool allowUnscheduled { opts.get<bool>("allowUnscheduled", false) };
  size_t nSchedules { opts.get<size_t>("num_schedules", 1) };
  // Check we're not being asked to do something we can't.
  if (allowUnscheduled && nSchedules > 1) {
    throw Exception(errors::UnimplementedFeature)
        << "Multi-schedule operation is not possible with on-demand "
        << "module execution.\n";
  }
  // Identify and process paths.
  std::set<std::string> known_pars {
    "analyzers",
    "filters",
    "producers",
    "trigger_paths",
    "end_paths"
  };
  ParameterSet empty;
  auto const physics = procPS_.get<ParameterSet>("physics", empty);
  auto const keys = physics.get_keys();
  vstring path_names;
  std::set_difference(keys.begin(),
                      keys.end(),
                      known_pars.begin(),
                      known_pars.end(),
                      std::back_inserter(path_names));
  std::set<std::string> specified_modules;
  // Process each path.
  for (auto const & path_name : path_names) {
    auto const path_pset = physics.get<ParameterSet>(path_name);
    if (processOnePathConfig_(path_name, path_pset)) {
      trigger_path_names.push_back(path_name);
    }
    auto const path_seq = path_pset.get_keys();
    specified_modules.insert(path_seq.cbegin(), path_seq.cend());
  }
  return std::move(trigger_path_names);
}

bool // Is trigger path.
art::PathManager::
processOnePathConfig_(std::string const & path_name __attribute__((unused)),
                      fhicl::ParameterSet const & path_pset)
{
  auto modnames = path_pset.get_keys();
  for (auto const & modname : modnames) {
    (void) modname;
  }
  // TODO: real return value.
  return true; // Path is a trigger path.
}
