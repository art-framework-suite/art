#include "art/Framework/Core/PathManager.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/ModuleBase.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/PathsInfo.h"
#include "art/Framework/Core/TriggerResultInserter.h"
#include "art/Framework/Core/WorkerInPath.h"
#include "art/Framework/Core/detail/ModuleGraphInfoMap.h"
#include "art/Framework/Core/detail/consumed_products.h"
#include "art/Framework/Core/detail/graph_algorithms.h"
#include "art/Framework/Core/fwd.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/ModuleType.h"
#include "art/Persistency/Provenance/PathSpec.h"
#include "art/Utilities/GlobalTaskGroup.h"
#include "art/Utilities/Globals.h"
#include "art/Utilities/PerScheduleContainer.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/ScheduleIteration.h"
#include "art/Utilities/TaskDebugMacros.h"
#include "art/Utilities/detail/remove_whitespace.h"
#include "art/Version/GetReleaseVersion.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/LibraryManager.h"
#include "cetlib/bold_fontify.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/detail/wrapLibraryManagerException.h"
#include "cetlib/ostream_handle.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "fhiclcpp/types/detail/validationException.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "range/v3/action.hpp"
#include "range/v3/view.hpp"

#include <algorithm>
#include <cassert>
#include <map>
#include <memory>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

using fhicl::ParameterSet;

namespace art {

  namespace {
    std::vector<std::string>
    sorted_module_labels(std::vector<WorkerInPath::ConfigInfo> const& wcis)
    {
      auto to_label = [](auto const& wci) {
        return wci.moduleConfigInfo->modDescription.moduleLabel();
      };
      using namespace ranges;
      return wcis | views::transform(to_label) | to<std::vector>() |
             ranges::actions::sort;
    }
  } // anonymous namespace

  PathManager::PathManager(ParameterSet const& procPS,
                           UpdateOutputCallbacks& outputCallbacks,
                           ProductDescriptions& productsToProduce,
                           ActionTable const& exceptActions,
                           ActivityRegistry const& actReg,
                           detail::EnabledModules const& enabled_modules)
    : outputCallbacks_{outputCallbacks}
    , exceptActions_{exceptActions}
    , actReg_{actReg}
    , procPS_{procPS}
    , triggerPathSpecs_{enabled_modules.trigger_path_specs()}
    , triggerPathsInfo_{Globals::instance()->nschedules()}
    , endPathInfo_(Globals::instance()->nschedules())
    , productsToProduce_{productsToProduce}
    , processName_{procPS.get<string>("process_name", {})}
    , allModules_{moduleInformation_(enabled_modules)}
    , triggerResultsWorkers_{Globals::instance()->nschedules()}
  {
    // Trigger paths
    auto const& trigger_path_specs = enabled_modules.trigger_path_specs();
    protoTrigPathLabels_.reserve(trigger_path_specs.size());
    std::set<std::string> recorded_path_name;
    for (auto const& [path_spec, entries] : trigger_path_specs) {
      if (not recorded_path_name.insert(path_spec.name).second)
        continue;

      if (entries.empty())
        continue;

      detail::configs_t worker_config_infos{};
      for (auto const& [label, action] : entries) {
        auto const& mci = allModules_.at(label);
        auto const mci_p = cet::make_exempt_ptr(&mci);
        worker_config_infos.emplace_back(mci_p, action);
      }
      protoTrigPathLabels_.emplace_back(path_spec,
                                        std::move(worker_config_infos));
    }

    // Finalize trigger path names and make available in Globals
    Globals::instance()->setProcessName(processName_);

    ParameterSet triggerPSet;
    triggerPSet.put("trigger_paths", prependedTriggerPathNames_());
    fhicl::ParameterSetRegistry::put(triggerPSet);
    Globals::instance()->setTriggerPSet(triggerPSet);
    Globals::instance()->setTriggerPathNames(triggerPathNames_());

    // End path(s)
    auto const& end_paths = enabled_modules.end_paths();
    protoEndPathLabels_.reserve(end_paths.size());
    for (auto const& [path_spec, entries] : end_paths) {
      if (not recorded_path_name.insert(path_spec.name).second)
        continue;

      if (entries.empty())
        continue;

      for (auto const& [label, action] : entries) {
        assert(action == detail::FilterAction::Normal);
        auto const& mci = allModules_.at(label);
        auto const mci_p = cet::make_exempt_ptr(&mci);
        protoEndPathLabels_.emplace_back(mci_p, action);
      }
    }

    if (size(end_paths) > 1u) {
      mf::LogInfo("PathConfiguration")
        << "Multiple end paths have been combined into one end path,\n"
        << "\"end_path\" since order is irrelevant.";
    }
  }

  std::vector<PathSpec>
  PathManager::triggerPathSpecs() const
  {
    using namespace ranges;
    return triggerPathSpecs_ | views::keys | to<std::vector>();
  }

  std::vector<std::string>
  PathManager::triggerPathNames_() const
  {
    using namespace ranges;
    return triggerPathSpecs_ | views::keys |
           views::transform([](auto const& spec) { return spec.name; }) |
           to<std::vector>();
  }

  std::vector<std::string>
  PathManager::prependedTriggerPathNames_() const
  {
    using namespace ranges;
    return triggerPathSpecs_ | views::keys |
           views::transform([](auto const& spec) { return to_string(spec); }) |
           to<std::vector>();
  }

  void
  PathManager::createModulesAndWorkers(
    GlobalTaskGroup& task_group,
    detail::SharedResources& resources,
    std::vector<std::string> const& producing_services)
  {
    // For each configured schedule, create the trigger paths and the
    // workers on each path.
    auto const nschedules =
      static_cast<ScheduleID::size_type>(Globals::instance()->nschedules());

    // The modules created are managed by shared_ptrs.  Once the
    // workers claim (co-)ownership of the modules, the 'modules'
    // object can be destroyed.
    modules_ = makeModules_(nschedules);

    // FIXME: THE PATHS INFO OBJECTS SHOULD BECOME OWNERS OF THE WORKERS
    //        I IMAGINE AN API LIKE:
    //
    //        pinfo.fillWorkers(pc, worker_config_infos, task_group, resources);
    //        pinfo.add_path(...);
    //
    //        PERHAPS WOULD BE BETTER SOMETHING LIKE:
    //
    //        Paths;
    //        wp.add_path(....);

    for (ScheduleID::size_type i = 0; i != nschedules; ++i) {
      ScheduleID const sid{i};
      auto& pinfo = triggerPathsInfo_[sid];
      ScheduleContext const sc{sid};
      for (auto const& [path_spec, worker_config_infos] :
           protoTrigPathLabels_) {

        PathContext const pc{
          sc, path_spec, sorted_module_labels(worker_config_infos)};
        auto wips = fillWorkers_(
          pc, worker_config_infos, pinfo.workers(), task_group, resources);
        pinfo.add_path(exceptActions_, actReg_, pc, move(wips), task_group);
      }

      if (protoEndPathLabels_.empty()) {
        continue;
      }

      // Create the end path and the workers on it.
      auto& einfo = endPathInfo_[sid];
      PathContext const pc{sc,
                           PathContext::end_path_spec(),
                           sorted_module_labels(protoEndPathLabels_)};
      auto wips = fillWorkers_(
        pc, protoEndPathLabels_, einfo.workers(), task_group, resources);
      einfo.add_path(exceptActions_, actReg_, pc, move(wips), task_group);
    }

    // Create trigger-results inserters
    PerScheduleContainer<std::unique_ptr<ModuleBase>> trigger_results(
      nschedules);
    for (ScheduleID::size_type i = 0; i != nschedules; ++i) {
      ScheduleID const sid{i};
      auto results_inserter = makeTriggerResultsInserter_(sid);
      if (!results_inserter)
        continue;

      WorkerParams const wp{outputCallbacks_,
                            productsToProduce_,
                            actReg_,
                            exceptActions_,
                            sid,
                            task_group.native_group(),
                            resources};
      triggerResultsWorkers_[sid] = results_inserter->makeWorker(wp);
      trigger_results[sid] = std::move(results_inserter);
    }

    modules_.replicated.emplace("TriggerResults", std::move(trigger_results));

    using namespace detail;
    auto const graph_info_collection =
      getModuleGraphInfoCollection_(producing_services);
    ModuleGraphInfoMap const modInfos{graph_info_collection};
    auto const module_graph =
      make_module_graph(modInfos, protoTrigPathLabels_, protoEndPathLabels_);
    auto const graph_filename =
      procPS_.get<string>("services.scheduler.dataDependencyGraph", {});
    if (!graph_filename.empty()) {
      cet::ostream_handle osh{graph_filename};
      print_module_graph(osh, modInfos, module_graph.first);
      cerr << "Generated data-dependency graph file: " << graph_filename
           << '\n';
    }
    auto const& err = module_graph.second;
    if (!err.empty()) {
      throw Exception{errors::Configuration} << err << '\n';
    }

    // No longer need worker/module config objects.
    protoTrigPathLabels_.clear();
    protoEndPathLabels_.clear();
    allModules_.clear();
  }

  PathsInfo&
  PathManager::triggerPathsInfo(ScheduleID const sid)
  {
    return triggerPathsInfo_.at(sid);
  }

  PerScheduleContainer<PathsInfo> const&
  PathManager::triggerPathsInfo()
  {
    return triggerPathsInfo_;
  }

  PathsInfo&
  PathManager::endPathInfo(ScheduleID const sid)
  {
    return endPathInfo_.at(sid);
  }

  PerScheduleContainer<PathsInfo> const&
  PathManager::endPathInfo()
  {
    return endPathInfo_;
  }

  std::map<std::string, detail::ModuleConfigInfo>
  PathManager::moduleInformation_(
    detail::EnabledModules const& enabled_modules) const
  {
    std::map<std::string, detail::ModuleConfigInfo> result{};
    ostringstream es;
    for (auto const& [module_label, key_and_type] : enabled_modules.modules()) {
      try {
        auto const& [key, module_type] = key_and_type;
        auto const module_pset = procPS_.get<fhicl::ParameterSet>(key);
        auto const lib_spec = module_pset.get<string>("module_type");
        if (auto const actual_mod_type = loadModuleType_(lib_spec);
            actual_mod_type != module_type) {
          es << "  ERROR: Module with label " << module_label << " of type "
             << lib_spec << " is configured as a " << to_string(module_type)
             << " but defined in code as a " << to_string(actual_mod_type)
             << ".\n";
          continue;
        }

        ModuleDescription const md{module_pset.id(),
                                   lib_spec,
                                   module_label,
                                   loadModuleThreadingType_(lib_spec),
                                   ProcessConfiguration{processName_,
                                                        procPS_.id(),
                                                        getReleaseVersion()}};
        detail::ModuleConfigInfo mci{md, std::move(module_pset), module_type};
        result.try_emplace(module_label, move(mci));
      }
      catch (exception const& e) {
        es << "  ERROR: Configuration of module with label " << module_label
           << " encountered the following error:\n"
           << e.what();
      }
    }
    if (auto err_msg = es.str(); not empty(err_msg)) {
      throw Exception(errors::Configuration)
        << "The following were encountered while processing the module "
           "configurations:\n"
        << err_msg;
    }
    return result;
  }

  PathManager::ModulesByThreadingType
  PathManager::makeModules_(ScheduleID::size_type const nschedules)
  {
    ModulesByThreadingType modules{};
    vector<string> configErrMsgs;
    for (auto const& [module_label, mci] : allModules_) {
      auto const& modPS = mci.modPS;
      auto const& md = mci.modDescription;
      auto const module_type = md.moduleName();
      auto const module_threading_type = md.moduleThreadingType();

      // FIXME: provide context information?
      actReg_.sPreModuleConstruction.invoke(md);

      auto sid = ScheduleID::first();
      auto mod = makeModule_(modPS, md, sid);
      if (auto err_msg = get_if<std::string>(&mod)) {
        configErrMsgs.push_back(*err_msg);
        continue;
      }

      assert(std::holds_alternative<ModuleBase*>(mod));
      auto module = std::get<ModuleBase*>(mod);

      if (module_threading_type == ModuleThreadingType::shared ||
          module_threading_type == ModuleThreadingType::legacy) {
        modules.shared.emplace(module_label,
                               std::unique_ptr<ModuleBase>{module});
      } else {
        PerScheduleContainer<std::unique_ptr<ModuleBase>> replicated_modules(
          nschedules);
        replicated_modules[sid].reset(module);
        ScheduleIteration schedule_iteration{sid.next(),
                                             ScheduleID(nschedules)};

        auto fill_replicated_module = [&, this](ScheduleID const sid) {
          auto repl_mod = makeModule_(modPS, md, sid);
          if (auto mod_ptr = get_if<ModuleBase*>(&repl_mod)) {
            replicated_modules[sid].reset(*mod_ptr);
          }
        };
        schedule_iteration.for_each_schedule(fill_replicated_module);
        modules.replicated.emplace(module_label, std::move(replicated_modules));
      }

      actReg_.sPostModuleConstruction.invoke(md);

      // Since we store consumes information per module label, we only
      // sort and collect it for one of the replicated-module copies.
      // The only way this would be a problem is if someone decided to
      // provided conditional consumes calls based on the ScheduleID
      // presented to the replicated-module constructor.
      module->sortConsumables(processName_);
      ConsumesInfo::instance()->collectConsumes(module_label,
                                                module->getConsumables());
    }

    if (!configErrMsgs.empty()) {
      constexpr cet::HorizontalRule rule{100};
      ostringstream msg;
      msg << "\n"
          << rule('=') << "\n\n"
          << "!! The following modules have been misconfigured: !!"
          << "\n";
      for (auto const& err : configErrMsgs) {
        msg << "\n" << rule('-') << "\n" << err;
      }
      msg << "\n" << rule('=') << "\n\n";
      throw Exception(errors::Configuration) << msg.str();
    }
    return modules;
  }

  PathManager::maybe_module_t
  PathManager::makeModule_(ParameterSet const& modPS,
                           ModuleDescription const& md,
                           ScheduleID const sid) const try {
    auto const& module_type = md.moduleName();
    detail::ModuleMaker_t* module_factory_func{nullptr};
    try {
      lm_.getSymbolByLibspec(module_type, "make_module", module_factory_func);
    }
    catch (Exception& e) {
      cet::detail::wrapLibraryManagerException(
        e, "Module", module_type, getReleaseVersion());
    }
    if (module_factory_func == nullptr) {
      throw Exception(errors::Configuration, "BadPluginLibrary: ")
        << "Module " << module_type << " with version " << getReleaseVersion()
        << " has internal symbol definition problems: consult an "
           "expert.";
    }
    auto mod = module_factory_func(modPS, ProcessingFrame{sid});
    mod->setModuleDescription(md);
    return mod;
  }
  catch (fhicl::detail::validationException const& e) {
    ostringstream es;
    es << "\n\nModule label: " << cet::bold_fontify(md.moduleLabel())
       << "\nmodule_type : " << cet::bold_fontify(md.moduleName()) << "\n\n"
       << e.what();
    return es.str();
  }

  std::unique_ptr<ReplicatedProducer>
  PathManager::makeTriggerResultsInserter_(ScheduleID const scheduleID)
  {
    auto& pathsInfo = triggerPathsInfo_.at(scheduleID);
    if (pathsInfo.paths().empty()) {
      return nullptr;
    }

    auto const& trig_pset = Globals::instance()->triggerPSet();
    ModuleDescription md{
      trig_pset.id(),
      "TriggerResultInserter",
      "TriggerResults",
      ModuleThreadingType::replicated,
      ProcessConfiguration{processName_, procPS_.id(), getReleaseVersion()}};
    actReg_.sPreModuleConstruction.invoke(md);
    auto producer = std::make_unique<TriggerResultInserter>(
      trig_pset, scheduleID, pathsInfo.pathResults());
    producer->setModuleDescription(md);
    actReg_.sPostModuleConstruction.invoke(md);
    return producer;
  }

  std::unique_ptr<Worker>
  PathManager::releaseTriggerResultsInserter(ScheduleID const scheduleID)
  {
    return std::move(triggerResultsWorkers_.at(scheduleID));
  }

  vector<WorkerInPath>
  PathManager::fillWorkers_(PathContext const& pc,
                            vector<WorkerInPath::ConfigInfo> const& wci_list,
                            map<string, std::shared_ptr<Worker>>& workers,
                            GlobalTaskGroup& task_group,
                            detail::SharedResources& resources)
  {
    auto const sid = pc.scheduleID();
    auto const pi = pc.pathID();
    vector<WorkerInPath> wips;
    for (auto const& wci : wci_list) {
      auto const& mci = *wci.moduleConfigInfo;
      auto const filterAction = wci.filterAction;
      auto const& module_label = mci.modDescription.moduleLabel();

      auto const& md = mci.modDescription;
      std::shared_ptr<Worker> worker{nullptr};
      // Workers present on multiple paths are shared so that their
      // work is only done once per schedule.
      if (auto it = workers.find(module_label); it != workers.end()) {
        TDEBUG_FUNC_SI(5, sid)
          << "Reusing worker " << hex << it->second << dec
          << " path: " << to_string(pi) << " type: " << md.moduleName()
          << " label: " << module_label;
        worker = it->second;
      } else {
        WorkerParams const wp{outputCallbacks_,
                              productsToProduce_,
                              actReg_,
                              exceptActions_,
                              sid,
                              task_group.native_group(),
                              resources};
        worker = makeWorker_(mci.modDescription, wp);
        TDEBUG(5) << "Made worker " << hex << worker << dec << " (" << sid
                  << ") path: " << to_string(pi) << " type: " << md.moduleName()
                  << " label: " << module_label << "\n";
      }

      assert(worker);
      workers.emplace(module_label, worker);
      wips.emplace_back(
        cet::make_exempt_ptr(worker.get()), filterAction, pc, task_group);
    }
    return wips;
  }

  std::shared_ptr<Worker>
  PathManager::makeWorker_(ModuleDescription const& md, WorkerParams const& wp)
  {
    auto get_module = [this](std::string const& module_label,
                             ModuleThreadingType const module_threading_type,
                             ScheduleID const sid) -> auto& {
      if (module_threading_type == ModuleThreadingType::shared ||
          module_threading_type == ModuleThreadingType::legacy) {
        return modules_.shared.at(module_label);
      }
      return modules_.replicated.at(module_label)[sid];
    };

    auto& module =
      get_module(md.moduleLabel(), md.moduleThreadingType(), wp.scheduleID_);
    return module->makeWorker(wp);
  }

  ModuleType
  PathManager::loadModuleType_(string const& lib_spec) const
  {
    detail::ModuleTypeFunc_t* mod_type_func{nullptr};
    try {
      lm_.getSymbolByLibspec(lib_spec, "moduleType", mod_type_func);
    }
    catch (Exception& e) {
      cet::detail::wrapLibraryManagerException(
        e, "Module", lib_spec, getReleaseVersion());
    }
    if (mod_type_func == nullptr) {
      throw Exception(errors::Configuration, "BadPluginLibrary")
        << "Module " << lib_spec << " with version " << getReleaseVersion()
        << " has internal symbol definition problems: consult an expert.";
    }
    return mod_type_func();
  }

  ModuleThreadingType
  PathManager::loadModuleThreadingType_(string const& lib_spec) const
  {
    detail::ModuleThreadingTypeFunc_t* mod_threading_type_func{nullptr};
    try {
      lm_.getSymbolByLibspec(
        lib_spec, "moduleThreadingType", mod_threading_type_func);
    }
    catch (Exception& e) {
      cet::detail::wrapLibraryManagerException(
        e, "Module", lib_spec, getReleaseVersion());
    }
    if (mod_threading_type_func == nullptr) {
      throw Exception(errors::Configuration, "BadPluginLibrary")
        << "Module " << lib_spec << " with version " << getReleaseVersion()
        << " has internal symbol definition problems: consult an expert.";
    }
    return mod_threading_type_func();
  }

  // Module-graph implementation below
  using namespace detail;

  collection_map_t
  PathManager::getModuleGraphInfoCollection_(
    std::vector<std::string> const& producing_services)
  {
    collection_map_t result{};
    auto& source_info = result["input_source"];
    if (!protoTrigPathLabels_.empty()) {
      set<string> path_names;
      for (auto const& spec : triggerPathSpecs_ | ranges::views::keys) {
        path_names.insert(spec.name);
      }
      source_info.paths = path_names;
      result["TriggerResults"] = ModuleGraphInfo{ModuleType::producer};
    } else if (!protoEndPathLabels_.empty()) {
      source_info.paths = {"end_path"};
    }

    // Prepare information for produced and viewable products
    std::map<std::string, std::set<ProductInfo>> produced_products_per_module;
    std::map<std::string, std::set<std::string>> viewable_products_per_module;
    for (auto const& pd : productsToProduce_) {
      auto const& module_name = pd.moduleLabel();
      produced_products_per_module[module_name].emplace(
        ProductInfo::ConsumableType::Product,
        pd.friendlyClassName(),
        pd.moduleLabel(),
        pd.productInstanceName(),
        ProcessTag{pd.processName(), pd.processName()});
      if (pd.supportsView()) {
        // We do not do any type-checking here due to lack of
        // introspection abilities.  That will be performed during the
        // actual product lookup.
        viewable_products_per_module[module_name].insert(
          pd.productInstanceName());
      }
    }

    // Handle producing services, which do not currently support 'consumes'.
    for (auto const& service_name : producing_services) {
      auto& graph_info = result[service_name];
      graph_info.module_type = ModuleType::producing_service;

      auto found = produced_products_per_module.find(service_name);
      if (found == cend(produced_products_per_module)) {
        continue;
      }

      graph_info.produced_products = found->second;
    }

    for (auto const& [path_spec, module_labels] : protoTrigPathLabels_) {
      fillModuleOnlyDeps_(path_spec.name,
                          module_labels,
                          produced_products_per_module,
                          viewable_products_per_module,
                          result);
    }
    fillModuleOnlyDeps_(PathContext::end_path(),
                        protoEndPathLabels_,
                        produced_products_per_module,
                        viewable_products_per_module,
                        result);
    fillSelectEventsDeps_(protoEndPathLabels_, result);
    return result;
  }

  void
  PathManager::fillModuleOnlyDeps_(
    string const& path_name,
    configs_t const& worker_configs,
    std::map<std::string, std::set<ProductInfo>> const& produced_products,
    std::map<std::string, std::set<std::string>> const& viewable_products,
    collection_map_t& info_collection) const
  {
    auto const worker_config_begin = cbegin(worker_configs);

    for (auto it = worker_config_begin, end = cend(worker_configs); it != end;
         ++it) {
      auto const& mci = *it->moduleConfigInfo;
      auto const& module_name = mci.modDescription.moduleLabel();
      auto& graph_info = info_collection[module_name];
      graph_info.paths.insert(path_name);
      graph_info.module_type = mci.moduleType;

      auto found = produced_products.find(module_name);
      if (found != cend(produced_products)) {
        graph_info.produced_products = found->second;
      }

      auto const& consumables =
        ConsumesInfo::instance()->consumables(module_name);
      graph_info.consumed_products =
        detail::consumed_products_for_module(processName_,
                                             consumables,
                                             produced_products,
                                             viewable_products,
                                             worker_config_begin,
                                             it);
    }
  }

  namespace {
    // The allowed path-specification is more restricted than what we
    // formulate here--i.e. a path name cannot begin with a digit.
    // However, because the FHiCL language does not allow parameter
    // names to begin with a digit, we do not worry about the overly
    // permissive regex below.
    string const allowed_path_spec{R"([\w\*\?]+)"};
    regex const regex{"(\\w+:)?(!|exception@)?(" + allowed_path_spec +
                      ")(&noexception)?"};
  }

  void
  PathManager::fillSelectEventsDeps_(configs_t const& worker_configs,
                                     collection_map_t& info_collection) const
  {
    for (auto const& worker_config : worker_configs) {
      auto const& mci = *worker_config.moduleConfigInfo;
      auto const& module_name = mci.modDescription.moduleLabel();
      auto const& ps = mci.modPS;
      auto& graph_info = info_collection[module_name];
      assert(is_observer(graph_info.module_type));
      auto path_specs = ps.get<vector<string>>("SelectEvents", {});
      for (auto& path_spec : path_specs) {
        detail::remove_whitespace(path_spec);

        smatch matches;
        regex_match(path_spec, matches, regex);
        // By the time we have gotten here, all modules have been
        // constructed, and it is guaranteed that the specified paths
        // are in accord with the above regex.
        //   0: Full match
        //   1: Optional process name
        //   2: Optional '!' or 'exception@'
        //   3: Required path specification
        //   4: Optional '&noexception'
        assert(matches.size() == 5);
        graph_info.select_events.insert(matches[3]);
      }
    }
  }

} // namespace art
