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
}

namespace art {

  class ActionTable;
  class ActivityRegistry;
  class ModuleBase;
  class UpdateOutputCallbacks;

  class PathManager {
  private:
    struct ModuleConfigInfo {
      std::string configTableName_;
      ModuleType moduleType_;
      ModuleThreadingType moduleThreadingType_;
      fhicl::ParameterSet modPS_;
      std::string libSpec_;
    };

  public:
    ~PathManager();

    PathManager(fhicl::ParameterSet const& procPS,
                UpdateOutputCallbacks& preg,
                ProductDescriptions& productsToProduce,
                ActionTable& exceptActions,
                ActivityRegistry& areg);

    PathManager(PathManager const&) = delete;
    PathManager(PathManager&&) = delete;
    PathManager& operator=(PathManager const&) = delete;
    PathManager& operator=(PathManager&&) = delete;

  public:
    std::vector<std::string> const& triggerPathNames() const;
    void createModulesAndWorkers();

    PathsInfo& triggerPathsInfo(ScheduleID scheduleID);
    PerScheduleContainer<PathsInfo>& triggerPathsInfo();
    PathsInfo& endPathInfo();

    Worker* triggerResultsInserter(ScheduleID scheduleID) const;
    void setTriggerResultsInserter(ScheduleID scheduleID,
                                   std::unique_ptr<WorkerT<EDProducer>>&&);

  private:
    void fillWorkers_(ScheduleID si,
                      int pi,
                      std::vector<WorkerInPath::ConfigInfo> const& wci_list,
                      std::vector<WorkerInPath>& wips,
                      std::map<std::string, Worker*>& workers);

    ModuleType loadModuleType_(std::string const& lib_spec);
    ModuleThreadingType loadModuleThreadingType_(std::string const& lib_spec);
    detail::collection_map_t getModuleGraphInfoCollection_();

    UpdateOutputCallbacks& outputCallbacks_;
    ActionTable& exceptActions_;
    ActivityRegistry& actReg_;
    cet::LibraryManager lm_{Suffixes::module()};
    fhicl::ParameterSet procPS_{};

    std::vector<std::string> triggerPathNames_{};

    // All unique module objects from any and all paths.
    std::map<module_label_t, ModuleBase*> moduleSet_{};

    // All unique worker objects from any and all paths.
    std::map<module_label_t, Worker*> workerSet_{};

    // Key is schedule number.
    PerScheduleContainer<PathsInfo> triggerPathsInfo_{};

    PathsInfo endPathInfo_{};
    PerScheduleContainer<std::unique_ptr<WorkerT<EDProducer>>> triggerResultsInserter_{};
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
