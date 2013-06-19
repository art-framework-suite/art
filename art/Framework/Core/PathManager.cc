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
  protoTrigPathMap_(),
  protoEndPathInfo_(),
  endPathWorkers_(),
  endPath_(),
  triggerPathWorkers_(),
  triggerPaths_(),
  triggerPathNames_(processPathConfigs_())
{
}

art::Path &
art::PathManager::
endPath()
{
  if (!endPath_) {
    // Need to create path from proto information.
    endPath_ = fillWorkers_(protoEndPathInfo_, endPathWorkers_);
  }
  return *endPath_;
}

art::PathManager::Paths const &
art::PathManager::
triggerPaths(ScheduleID sID)
{
  if (triggerPathNames_.empty()) {
    return triggerPaths_[sID]; // Empty.
  } else {
    auto it =
      triggerPaths_.find(sID);
    if (it == triggerPaths_.end()) {
      // FIXME: use emplace().
      it = triggerPaths_.insert(std::make_pair(sID, Paths())).first;
      std::for_each(protoTrigPathMap_.cbegin(),
                    protoTrigPathMap_.cbegin(),
                    [this, sID, it](typename decltype(protoTrigPathMap_)::value_type const & val)
                    {
                      it->second.emplace_back(fillWorkers_(val.second, triggerPathWorkers_[sID]));
                    });
    }
    return it->second;
  }
}

art::detail::ModuleConfigInfoMap
art::PathManager::
fillAllModules_()
{
  static ParameterSet const empty;
  detail::ModuleConfigInfoMap all_modules;
  for (auto const & pathRootName :
       detail::ModuleConfigInfo::allModulePathRoots()) {
    auto const pathRoot = procPS_.get<ParameterSet>(pathRootName, empty);
    auto const names = pathRoot.get_keys();
    for (auto const & name : names) {
      detail::ModuleConfigInfo mci(procPS_, name, pathRootName);
      auto actualModType = fact_.moduleType(mci.libSpec());
      if (actualModType != mci.moduleType()) {
        throw Exception(errors::Configuration)
            << "Module with label "
            << mci.label()
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
                           value_type(mci.label(), mci));
      if (!result.second) {
        throw Exception(errors::Configuration)
            << "Module label "
            << mci.label()
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
  auto nSchedules = opts.get<size_t>("num_schedules", 1);
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
    auto const path_seq = physics.get<vstring>(path_name);
    if (processOnePathConfig_(path_name, path_seq)) {
      trigger_path_names.push_back(path_name);
    }
    specified_modules.insert(path_seq.cbegin(), path_seq.cend());
  }
  return std::move(trigger_path_names);
}

bool // Is trigger path.
art::PathManager::
processOnePathConfig_(std::string const & path_name __attribute__((unused)),
                      vstring const & path_seq)
{
  enum class mod_cat_t { UNSET, OBSERVER, MODIFIER };
  mod_cat_t cat { mod_cat_t::UNSET };
  for (auto const & modname : path_seq) {
    auto const label = detail::ModuleConfigInfo::stripLabel(modname);
    auto const it = allModules_.find(label);
    if (it == allModules_.end()) {
      throw Exception(errors::Configuration)
        << "Entry "
        << modname
        << " in path "
        << path_name
        << " refers to a module label "
        << label
        << " which is not configured.\n";
    }
    auto mtype = is_observer(it->second.moduleType()) ?
                 mod_cat_t::OBSERVER :
                 mod_cat_t::MODIFIER;
    if (cat == mod_cat_t::UNSET) {
      cat = mtype;
      // Efficiency.
      if (cat == mod_cat_t::MODIFIER) {
        protoTrigPathMap_[path_name].reserve(path_seq.size());
      }
      else {
        protoEndPathInfo_.reserve(protoEndPathInfo_.size() +
                                   path_seq.size());
      }
    }
    else if (cat != mtype) {
      throw Exception(errors::Configuration)
        << "Entry "
        << modname
        << " in path "
        << path_name
        << " is a"
        << (cat == mod_cat_t::OBSERVER ? " modifier" : "n observer")
        << " while previous entries in the same path are all "
        << (cat == mod_cat_t::OBSERVER ? "observers" : "modifiers")
        << ".\n";
    }
    if (cat == mod_cat_t::MODIFIER) {
      protoTrigPathMap_[path_name].push_back(&it->second);
    } else { // Only one end path.
      protoEndPathInfo_.push_back(&it->second);
    }
  }
  return (cat == mod_cat_t::MODIFIER); // Path is a trigger path.
}

std::unique_ptr<art::Path>
art::PathManager::
fillWorkers_(ModInfos const & modInfos __attribute__((unused)),
             WorkerMap & workers __attribute__((unused)))
{
  // FIXME: do something real!
  return std::unique_ptr<art::Path>();
}
