#include "art/Framework/Core/PathManager.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Version/GetReleaseVersion.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/GetPassID.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/types/detail/validationException.h"

using fhicl::ParameterSet;

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <sstream>

namespace {
  std::unique_ptr<std::set<std::string> >
  findLegacyConfig(fhicl::ParameterSet const & ps,
                   std::string const & param)
  {
    std::vector<std::string> tmp;
    std::unique_ptr<std::set<std::string> > result;
    if (ps.get_if_present(param, tmp)) {
      result.reset(new std::set<std::string>);
      result->insert(tmp.cbegin(), tmp.cend());
    }
    return std::move(result);
  }

  std::string
  stripLabel(std::string const & labelInPathConfig)
  {
    auto label_start = labelInPathConfig.find_first_not_of("!-");
    if (label_start > 1) {
      throw art::Exception(art::errors::Configuration)
        << "Module label "
        << labelInPathConfig
        << " is illegal.\n";
    }
    return labelInPathConfig.substr(label_start);
  }

  art::WorkerInPath::FilterAction
  filterAction(std::string const & labelInPathConfig)
  {
    switch (labelInPathConfig[0]) {
    case '!':
      return art::WorkerInPath::FilterAction::Veto;
    case '-':
      return art::WorkerInPath::FilterAction::Ignore;
    default:
      return art::WorkerInPath::FilterAction::Normal;
    }
  }

  void
  check_missing_paths(std::set<std::string> const & specified_paths,
                      art::PathManager::vstring const & path_names,
                      std::string const & par_name,
                      std::ostream & error_stream)
  {
    art::PathManager::vstring missing_paths;
    std::set_difference(specified_paths.cbegin(),
                        specified_paths.cend(),
                        path_names.cbegin(),
                        path_names.cend(),
                        std::back_inserter(missing_paths));
    for (auto const & path : missing_paths) {
      error_stream
        << "ERROR: Unknown path "
        << path
        << " specified by user in "
        << par_name
        << ".\n";
    }
  }

  void
  check_misspecified_paths(fhicl::ParameterSet const & pset,
                           std::vector<std::string> const & path_names)
  {
    using name_t  = std::string;
    using fhicl_t = std::string;

    std::map<name_t,fhicl_t> bad_names;
    for ( auto const& name : path_names ){
      if ( pset.is_key_to_sequence(name) ) continue;
      std::string const type = pset.is_key_to_table(name) ? "table" : "atom";
      bad_names.emplace(name,type);
    }

    if ( bad_names.empty() ) return;

    std::string err_msg = "\n"
      "You have specified the following unsupported parameters in the\n"
      "\"physics\" block of your configuration:\n\n";

    cet::for_all(bad_names,
                 [&err_msg](auto const& name)
                 {
                   err_msg.append("   \"physics." +name.first+ "\"   ("+name.second+")\n");
                 } );

    err_msg
      .append("\n")
      .append("Supported parameters include the following tables:\n")
      .append("   \"physics.producers\"\n")
      .append("   \"physics.filters\"\n")
      .append("   \"physics.analyzers\"\n")
      .append("and sequences.  Atomic configuration parameters are not allowed.\n\n");

    throw art::Exception(art::errors::Configuration) << err_msg;
  }

}

art::PathManager::
PathManager(ParameterSet const & procPS,
            MasterProductRegistry & preg,
            ActionTable & exceptActions,
            ActivityRegistry & areg)
  :
  procPS_(procPS),
  preg_(preg),
  exceptActions_(exceptActions),
  areg_(areg),
  allowUnscheduled_(procPS_.get<bool>("services.scheduler.allowUnscheduled",
                                      false)),
  trigger_paths_config_(findLegacyConfig(procPS_, "physics.trigger_paths")),
  end_paths_config_(findLegacyConfig(procPS_, "physics.end_paths")),
  fact_(),
  allModules_(fillAllModules_()),
  protoTrigPathMap_(),
  protoEndPathInfo_(),
  triggerPathNames_(processPathConfigs_()),
  endPathInfo_(),
  triggerPathsInfo_(),
  configErrMsgs_()
{
}

art::PathsInfo &
art::PathManager::
endPathInfo()
{
  if (!protoEndPathInfo_.empty() &&
      endPathInfo_.pathPtrs().empty()) {
    // Need to create path from proto information.
    endPathInfo_.pathPtrs().emplace_back
      (fillWorkers_(0,
                    "end_path",
                    protoEndPathInfo_,
                    nullptr, // End path, no trigger results needed.
                    endPathInfo_.workers()));
  }
  return endPathInfo_;
}

art::PathsInfo &
art::PathManager::
triggerPathsInfo(ScheduleID sID)
{
  if (triggerPathNames_.empty())
    return triggerPathsInfo_[sID]; // Empty.

  auto it = triggerPathsInfo_.find(sID);
  if (it == triggerPathsInfo_.end()) {
    it = triggerPathsInfo_.emplace(sID, PathsInfo()).first;
    it->second.pathResults() = HLTGlobalStatus(triggerPathNames_.size());
    int bitpos { 0 };
    cet::for_all(protoTrigPathMap_,
                 [this, sID, it, &bitpos](auto const & val)
                 {
                   it->second.pathPtrs().emplace_back(this->fillWorkers_(bitpos,
                                                                         val.first,
                                                                         val.second,
                                                                         Path::TrigResPtr(&it->second.pathResults()),
                                                                         it->second.workers()));
                   ++bitpos;
                 });
  }
  return it->second;
}

art::PathManager::Workers
art::PathManager::
onDemandWorkers()
{
  Workers result;
  // FIXME: until we go truly multi-schedule and can deal with
  // multiply-constructed or shared modules, glom onto the primary
  // schedule's worker map.
  if (allowUnscheduled()) {
    for (auto const & val : allModules_) {
      if (is_modifier(val.second.moduleType())) {
        result.push_back(makeWorker_(val.second,
                                     triggerPathsInfo(ScheduleID::first()).workers()));
      }
    }
  }
  return std::move(result);
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
    for (auto const & name : pathRoot.get_names()) {
      try {
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
        auto result = all_modules.emplace(mci.label(), mci);
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
      catch (std::exception const &e ){
        error_stream
          << "  ERROR: Configuration of module with label "
          << name
          << " encountered the following error:\n"
          << e.what()
          << "\n";
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
  auto const keys = physics.get_names();
  vstring path_names;
  std::set_difference(keys.cbegin(),
                      keys.cend(),
                      known_pars.cbegin(),
                      known_pars.cend(),
                      std::back_inserter(path_names));
  std::set<std::string> specified_modules;
  std::ostringstream error_stream;

  // Check for missing specified paths.
  if (trigger_paths_config_) {
    check_missing_paths(*trigger_paths_config_,
                        path_names,
                        "trigger_paths",
                        error_stream);
  }
  if (end_paths_config_) {
    check_missing_paths(*end_paths_config_,
                        path_names,
                        "end_paths",
                        error_stream);
  }

  check_misspecified_paths(physics, path_names);

  // Process each path.
  size_t num_end_paths { 0 };
  for (auto const & path_name : path_names) {
    auto const path_seq = physics.get<vstring>(path_name);
    if (processOnePathConfig_(path_name,
                              path_seq,
                              trigger_path_names,
                              error_stream)) {
      ++num_end_paths;
    }
    for (auto const & mod : path_seq) {
      specified_modules.insert(stripLabel(mod));
    }
  }
  if (num_end_paths > 1) {
    mf::LogInfo("PathConfiguration")
      << "Multiple end paths have been combined into one end path,\n"
      << "\"end_path\" since order is irrelevant.\n";
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
                 << " not assigned to any path:\n"
                 << "'" << unused_modules.front() << "'";
    for (auto i = unused_modules.cbegin() + 1,
              e = unused_modules.cend();
         i != e;
         ++i) {
      unusedStream << ", '" << *i << "'";
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

bool // Is wanted end path.
art::PathManager::
processOnePathConfig_(std::string const & path_name,
                      vstring const & path_seq,
                      vstring & trigger_path_names,
                      std::ostream & error_stream)
{
  enum class mod_cat_t { UNSET, OBSERVER, MODIFIER };
  mod_cat_t cat { mod_cat_t::UNSET };
  for (auto const & modname : path_seq) {
    auto const label = stripLabel(modname);
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
      continue;
    }
    auto mtype = is_observer(it->second.moduleType()) ?
                 mod_cat_t::OBSERVER :
                 mod_cat_t::MODIFIER;
    if (cat == mod_cat_t::UNSET) {
      cat = mtype;
      // Efficiency.
      if (cat == mod_cat_t::MODIFIER) {
        if (trigger_paths_config_ &&
            (trigger_paths_config_->find(path_name) ==
             trigger_paths_config_->cend())) {
          mf::LogInfo("DeactivatedPath")
            << "Detected trigger path \""
            << path_name
            << "\" which was not found in\n"
            << "parameter \"physics.trigger_paths\". "
            << "Path will be ignored.";
          return false;
        }
        trigger_path_names.push_back(path_name);
        protoTrigPathMap_[path_name].reserve(path_seq.size());
      }
      else {
        if (end_paths_config_ &&
            (end_paths_config_->find(path_name) ==
             end_paths_config_->cend())) {
          mf::LogInfo("DeactivatedPath")
            << "Detected end path \""
            << path_name
            << "\" which was not found in\n"
            << "parameter \"physics.end_paths\". "
            << "Path will be ignored.";
          return false;
        }
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
      protoTrigPathMap_[path_name].emplace_back(it->second, filterAction(modname));
    } else { // Only one end path.
      protoEndPathInfo_.emplace_back(it->second, filterAction(modname));
    }
  }
  return (cat == mod_cat_t::OBSERVER);
}

void
art::PathManager::
makeWorker_(detail::ModuleInPathInfo const & mipi,
            WorkerMap & workers,
            std::vector<WorkerInPath> & pathWorkers)
{
  auto w = makeWorker_(mipi.moduleConfigInfo(), workers);
  pathWorkers.emplace_back(w, mipi.filterAction());
}

art::Worker *
art::PathManager::
makeWorker_(detail::ModuleConfigInfo const & mci,
            WorkerMap & workers)
{
  auto it = workers.find(mci.label());
  if (it == workers.end()) { // Need worker.
    auto moduleConfig = procPS_.get<ParameterSet>(mci.configPath() + '.' + mci.label());
    WorkerParams p(procPS_,
                   moduleConfig,
                   preg_,
                   exceptActions_,
                   art::ServiceHandle<art::TriggerNamesService>()->getProcessName());
    ModuleDescription md(moduleConfig.id(),
                         p.pset_.get<std::string>("module_type"),
                         p.pset_.get<std::string>("module_label"),
                         ProcessConfiguration(p.processName_,
                                              procPS_.id(),
                                              getReleaseVersion(),
                                              getPassID()));
    areg_.sPreModuleConstruction.invoke(md);
    try {
      auto worker = fact_.makeWorker(p, md);
      areg_.sPostModuleConstruction.invoke(md);
      it = workers.
        emplace(mci.label(),
                std::move(worker)).first;
      it->second->setActivityRegistry(&areg_);
    }
    catch ( fhicl::detail::validationException const & e ) {
      std::ostringstream err_stream;
      err_stream << "\n\nModule label: \033[1m" << md.moduleLabel() << "\033[0m"
                 <<   "\nmodule_type : \033[1m" << md.moduleName() <<  "\033[0m"
                 << "\n\n" << e.what();
      configErrMsgs_.push_back( err_stream.str() );
    }
  }
  return it->second.get();
}

// Precondition: !modInfos.empty();
std::unique_ptr<art::Path>
art::PathManager::
fillWorkers_(int bitpos,
             std::string const & pathName,
             ModInfos const & modInfos,
             Path::TrigResPtr pathResults,
             WorkerMap & workers)
{
  assert(!modInfos.empty());
  std::vector<WorkerInPath> pathWorkers;
  std::ostringstream config_error_stream;
  for (auto const & mci : modInfos) {
    makeWorker_(mci, workers, pathWorkers);
  }

  if ( configErrMsgs_.size() ) {
    std::size_t const width (100);
    std::ostringstream err_msg;
    err_msg << "\n"
            << std::string(width,'=')
            << "\n\n"
            << "!! The following modules have been misconfigured: !!"
            << "\n";
    for ( auto const& err : configErrMsgs_ ) {
      err_msg << "\n"
              << std::string(width,'-')
              << "\n"
              << err;
    }
    err_msg << "\n"
            << std::string(width,'=')
            << "\n\n";

    throw art::Exception(art::errors::Configuration) << err_msg.str();

  }

  return std::unique_ptr<art::Path>
    (new art::Path(bitpos,
                   pathName,
                   std::move(pathWorkers),
                   std::move(pathResults),
                   procPS_,
                   exceptActions_,
                   areg_,
                   is_observer(modInfos.front().moduleConfigInfo().moduleType())));
}
