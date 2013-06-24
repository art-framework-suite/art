#include "art/Framework/Core/PathManager.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Version/GetReleaseVersion.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/GetPassID.h"

using fhicl::ParameterSet;

#include <algorithm>
#include <map>
#include <set>
#include <sstream>
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
  allowUnscheduled_(procPS_.get<bool>("services.scheduler.allowUnscheduled",
                                      false)),
  fact_(),
  allModules_(fillAllModules_()),
  protoTrigPathMap_(),
  protoEndPathInfo_(),
  triggerPathNames_(processPathConfigs_()),
  endPathInfo_(),
  triggerPathsInfo_()
{
}

art::Path &
art::PathManager::
endPath()
{
  if (endPathInfo_.pathPtrs.empty()) {
    // Need to create path from proto information.
    endPathInfo_.pathResults = HLTGlobalStatus(1);
    endPathInfo_.pathPtrs.emplace_back
      (fillWorkers_(0,
                    "end_path",
                    protoEndPathInfo_,
                    endPathInfo_.pathResults,
                    endPathInfo_.workers));
  }
  return *endPathInfo_.pathPtrs.front();
}

art::PathPtrs const &
art::PathManager::
triggerPathPtrs(ScheduleID sID)
{
  if (triggerPathNames_.empty()) {
    return triggerPathsInfo_[sID].pathPtrs; // Empty.
  } else {
    auto it =
      triggerPathsInfo_.find(sID);
    if (it == triggerPathsInfo_.end()) {
      it = triggerPathsInfo_.emplace(sID, PathsInfo()).first;
      int bitpos { 0 };
      std::for_each(protoTrigPathMap_.cbegin(),
                    protoTrigPathMap_.cend(),
                    [this, sID, it, &bitpos](typename decltype(protoTrigPathMap_)::value_type const & val)
                    {
                      it->second.pathResults = HLTGlobalStatus(triggerPathNames_.size());
                      it->second.pathPtrs.emplace_back(fillWorkers_(bitpos,
                                                                    val.first,
                                                                    val.second,
                                                                    it->second.pathResults,
                                                                    it->second.workers));
                      ++bitpos;
                    });
    }
    return it->second.pathPtrs;
  }
}

art::detail::ModuleConfigInfoMap
art::PathManager::
fillAllModules_()
{
  static ParameterSet const empty;
  detail::ModuleConfigInfoMap all_modules;
  std::ostringstream error_stream;
  for (auto const & pathRootName :
       detail::ModuleConfigInfo::allModulePathRoots()) {
    auto const pathRoot = procPS_.get<ParameterSet>(pathRootName, empty);
    auto const names = pathRoot.get_keys();
    for (auto const & name : names) {
      detail::ModuleConfigInfo mci(procPS_, name, pathRootName);
      auto actualModType = fact_.moduleType(mci.libSpec());
      if (actualModType != mci.moduleType()) {
        error_stream
            << "  ERROR: Module with label "
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
        error_stream
            << "  ERROR: Module label "
            << mci.label()
            << " has been used in "
            << result.first->second.configPath()
            << " and "
            << pathRootName
            << ".\n";
      }
    }
  }
  auto error_messages = error_stream.str();
  if (!error_messages.empty()) {
    throw Exception(errors::Configuration)
      << "The following were encountered while processing the module configurations:\n"
      << error_messages;
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
  auto nSchedules = opts.get<size_t>("num_schedules", 1);
  // Check we're not being asked to do something we can't.
  if (allowUnscheduled_ && nSchedules > 1) {
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
  std::set_difference(keys.cbegin(),
                      keys.cend(),
                      known_pars.cbegin(),
                      known_pars.cend(),
                      std::back_inserter(path_names));
  std::set<std::string> specified_modules;
  // Process each path.
  std::ostringstream error_stream;
  size_t end_paths { 0 };
  for (auto const & path_name : path_names) {
    auto const path_seq = physics.get<vstring>(path_name);
    if (processOnePathConfig_(path_name, path_seq, error_stream)) { // Trigger
      trigger_path_names.push_back(path_name);
    }
    else { // End path
      ++end_paths;
    }
    if (end_paths > 1) {
      mf::LogInfo("PathConfiguration")
        << "Multiple end paths have been combined into one end path,\n"
        << " \"end_path\" since order is irrelevant.\n";
    }
    specified_modules.insert(path_seq.cbegin(), path_seq.cend());
  }

  if (allowUnscheduled_) {
    // All modifiers are "used."
    for (auto const & val : allModules_) {
      if (is_modifier(val.second.moduleType())) {
        specified_modules.emplace(val.second.label());
      }
    };
  }

  vstring unused_modules;
  std::set<std::string> all_module_labels;
  for(auto const & val : allModules_) {
    all_module_labels.insert(val.first);
  }

  std::set_difference(all_module_labels.cbegin(),
                      all_module_labels.cend(),
                      specified_modules.cbegin(),
                      specified_modules.cend(),
                      std::back_inserter(unused_modules));

  // Complain about unused modules.
  if (!unused_modules.empty()) {
    std::ostringstream unusedStream;
    unusedStream << "The following module label"
                 << ((unused_modules.size() == 1) ? " is" : "s are")
                 << "s are not assigned to any path:\n"
                 << "'" << unused_modules.front() << "'";
    for (auto i = unused_modules.cbegin() + 1,
              e = unused_modules.cend();
         i != e;
         ++i) {
      unusedStream << ", " << *i << "'";
    }
    mf::LogInfo("path")
      << unusedStream.str()
      << "\n";
  }

  // Check for fatal errors.
  auto error_messages = error_stream.str();
  if (!error_messages.empty()) {
    throw Exception(errors::Configuration, "Path configuration: ")
      << "The following were encountered while processing path configurations:\n"
      << error_messages;
  }
  return std::move(trigger_path_names);
}

bool // Is trigger path.
art::PathManager::
processOnePathConfig_(std::string const & path_name,
                      vstring const & path_seq,
                      std::ostream & error_stream)
{
  enum class mod_cat_t { UNSET, OBSERVER, MODIFIER };
  mod_cat_t cat { mod_cat_t::UNSET };
  for (auto const & modname : path_seq) {
    auto const label = detail::ModuleConfigInfo::stripLabel(modname);
    auto const it = allModules_.find(label);
    if (it == allModules_.end()) {
      error_stream
        << "  ERROR: Entry "
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
      error_stream
        << "  ERROR: Entry "
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

void
art::PathManager::
makeWorker_(detail::ModuleConfigInfo const * mci,
            WorkerMap & workers,
            std::vector<WorkerInPath> & pathWorkers)
{
  auto it = workers.find(mci->label());
  if (it == workers.end()) { // Need workers.
    WorkerParams p(procPS_,
                   procPS_.get<ParameterSet>(mci->configPath() + '.' + mci->label()),
                   preg_,
                   exceptActions_,
                   art::ServiceHandle<art::TriggerNamesService>()->getProcessName());
    ModuleDescription md(procPS_.id(),
                         p.pset_.get<std::string>("module_type"),
                         p.pset_.get<std::string>("module_label"),
                         ProcessConfiguration(p.processName_,
                                              procPS_.id(),
                                              getReleaseVersion(),
                                              getPassID()));
    areg_->sPreModuleConstruction.invoke(md);
    auto worker = fact_.makeWorker(p, md);
    areg_->sPostModuleConstruction.invoke(md);
    it = workers.
         emplace(mci->label(),
                 std::move(worker)).first;
    it->second->setActivityRegistry(areg_);
    pathWorkers.emplace_back(it->second.get());
  }
}

std::unique_ptr<art::Path>
art::PathManager::
fillWorkers_(int bitpos,
             std::string const & pathName,
             ModInfos const & modInfos,
             HLTGlobalStatus & pathResults,
             WorkerMap & workers)
{
  std::vector<WorkerInPath> pathWorkers;
  std::for_each(modInfos.cbegin(),
                modInfos.cend(),
                std::bind(&art::PathManager::makeWorker_,
                          this,
                          std::placeholders::_1,
                          workers,
                          pathWorkers)
               );
  return std::unique_ptr<art::Path>
    (new art::Path(bitpos,
                   pathName,
                   std::move(pathWorkers),
                   pathResults,
                   procPS_,
                   exceptActions_,
                   areg_,
                   is_observer(modInfos.front()->moduleType())));
}
