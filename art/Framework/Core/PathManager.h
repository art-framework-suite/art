#ifndef art_Framework_Core_PathManager_h
#define art_Framework_Core_PathManager_h
// vim: set sw=2 expandtab :

//
// PathManager.
//
// Class to handle the processing of the configuration of modules in
// art, including the creation of paths and construction of modules as
// appropriate.
//
// Intended to be constructed early, prior to services, since
// TriggerNamesService will need some of the information herein at
// construction time.
//

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleType.h"
#include "art/Framework/Core/PathsInfo.h"
#include "art/Framework/Core/WorkerInPath.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/ModuleGraphInfoMap.h"
#include "art/Utilities/PerScheduleContainer.h"
#include "art/Utilities/PluginSuffixes.h"
#include "art/Utilities/ScheduleID.h"
#include "cetlib/LibraryManager.h"
#include "fhiclcpp/ParameterSet.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace cet {
  class ostream_handle;
} // namespace cet

namespace art {
  class ActionTable;
  class ActivityRegistry;
  class ModuleBase;
  class UpdateOutputCallbacks;
  class PathManager {
  private: // Types
    struct ModuleConfigInfo {
      std::string configTableName_;
      ModuleType moduleType_;
      ModuleThreadingType moduleThreadingType_;
      fhicl::ParameterSet modPS_;
      std::string libSpec_;
    };

  public: // Special Member Functions
    ~PathManager();
    PathManager(fhicl::ParameterSet const& procPS,
                UpdateOutputCallbacks& preg,
                ProductDescriptions& productsToProduce,
                ActionTable const& exceptActions,
                ActivityRegistry const& areg);

    PathManager(PathManager const&) = delete;
    PathManager(PathManager&&) = delete;
    PathManager& operator=(PathManager const&) = delete;
    PathManager& operator=(PathManager&&) = delete;

  public: // API
    std::vector<std::string> const& triggerPathNames() const;
    void createModulesAndWorkers();
    PathsInfo& triggerPathsInfo(ScheduleID const);
    PerScheduleContainer<PathsInfo>& triggerPathsInfo();
    PathsInfo& endPathInfo();
    Worker* triggerResultsInserter(ScheduleID const) const;
    void setTriggerResultsInserter(
      ScheduleID const,
      std::unique_ptr<WorkerT<ReplicatedProducer>>&&);

  private: // Implementation Details
    void fillWorkers_(ScheduleID const,
                      int pi,
                      std::vector<WorkerInPath::ConfigInfo> const& wci_list,
                      std::vector<WorkerInPath>& wips,
                      std::map<std::string, Worker*>& workers);
    ModuleType loadModuleType_(std::string const& lib_spec);
    ModuleThreadingType loadModuleThreadingType_(std::string const& lib_spec);
    detail::collection_map_t getModuleGraphInfoCollection_();

  private: // Member Data
    UpdateOutputCallbacks& outputCallbacks_;
    ActionTable const& exceptActions_;
    ActivityRegistry const& actReg_;
    cet::LibraryManager lm_{Suffixes::module()};
    fhicl::ParameterSet procPS_{};
    std::vector<std::string> triggerPathNames_{};
    std::map<module_label_t, ModuleBase*> moduleSet_{};
    // FIXME: The number of workers is the number of schedules times
    //        the number of configured modules.  For a replicated
    //        module, there is one worker per module copy; for a
    //        shared module, there are as many workers as their are
    //        schedules.  To avoid any memory leaks, we use a multimap
    //        so we can delete the memory in the destructor.  This
    //        part of the code could benefit from using smart
    //        pointers.
    std::multimap<module_label_t, Worker*> workerSet_{};
    PerScheduleContainer<PathsInfo> triggerPathsInfo_{};
    PathsInfo endPathInfo_{};
    PerScheduleContainer<std::unique_ptr<WorkerT<ReplicatedProducer>>>
      triggerResultsInserter_{};
    ProductDescriptions& productsToProduce_;
    //  The following data members are only needed to delay the
    //  creation of modules until after the service system has
    //  started.  We can move them back to the ctor once that is
    //  fixed.
    std::string processName_{};
    std::map<std::string, ModuleConfigInfo> allModules_{};
    std::unique_ptr<std::set<std::string>> trigger_paths_config_{};
    std::unique_ptr<std::set<std::string>> end_paths_config_{};
    std::map<std::string, std::vector<WorkerInPath::ConfigInfo>>
      protoTrigPathLabelMap_{};
    std::vector<WorkerInPath::ConfigInfo> protoEndPathLabels_{};
  };
} // namespace art

#endif /* art_Framework_Core_PathManager_h */

// Local Variables:
// mode: c++
// End:
