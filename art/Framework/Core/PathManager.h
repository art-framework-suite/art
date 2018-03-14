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
#include "art/Framework/Core/ModuleBase.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/ModuleType.h"
#include "art/Framework/Core/Path.h"
#include "art/Framework/Core/PathsInfo.h"
#include "art/Framework/Core/WorkerInPath.h"
#include "art/Framework/Core/WorkerT.h"
// #include "art/Framework/Core/detail/ModuleGraph.h"
// #include "art/Framework/Core/detail/ModuleInfoMap.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Utilities/PluginSuffixes.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Version/GetReleaseVersion.h"
#include "canvas/Persistency/Common/HLTGlobalStatus.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/LibraryManager.h"
#include "cetlib/detail/wrapLibraryManagerException.h"
#include "cetlib/exempt_ptr.h"
#include "fhiclcpp/ParameterSet.h"

#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace cet {
  class ostream_handle;
}

namespace art {

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

    using module_label_t = std::string;

    struct WorkerConfigInfo {
      WorkerConfigInfo(module_label_t const& label,
                       WorkerInPath::FilterAction const filterAction) :
        label_{label},
        filterAction_{filterAction}
      {}

      module_label_t label_; // Used for looking up ModuleConfig
      WorkerInPath::FilterAction filterAction_;
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
    void createModulesAndWorkers(std::string const& debug_filename);

    PathsInfo& triggerPathsInfo(int stream);
    std::vector<PathsInfo>& triggerPathsInfo();
    PathsInfo& endPathInfo();

    Worker* triggerResultsInserter(int streamIndex) const;
    void setTriggerResultsInserter(int streamIndex,
                                   std::unique_ptr<WorkerT<EDProducer>>&&);

  private:

    void fillWorkers_(int si,
                      int pi,
                      std::vector<WorkerConfigInfo> const& wci_list,
                      std::map<std::string, Worker*>& allStreamWorkers,
                      std::vector<WorkerInPath>& wips,
                      std::map<std::string, Worker*>& workers);

    UpdateOutputCallbacks& outputCallbacks_;
    ActionTable& exceptActions_;
    ActivityRegistry& actReg_;
    cet::LibraryManager lm_{Suffixes::module()};
    fhicl::ParameterSet procPS_{};

    std::vector<std::string> triggerPathNames_{};

    // All unique module objects from any and all paths.
    std::multimap<module_label_t, ModuleBase*> moduleSet_{};

    // All unique worker objects from any and all paths.
    std::multimap<module_label_t, Worker*> workerSet_{};

    // Key is stream number.
    std::vector<PathsInfo> triggerPathsInfo_{};

    PathsInfo endPathInfo_{};
    std::vector<std::unique_ptr<WorkerT<EDProducer>>> triggerResultsInserter_{};
    ProductDescriptions& productsToProduce_;

    //  The following data members are only needed to delay the
    //  creation of modules until after the service system has
    //  started.  We can move them back to the ctor once that is
    //  fixed.

    std::string processName_{};
    std::map<std::string, ModuleConfigInfo> allModules_{};
    std::unique_ptr<std::set<std::string>> trigger_paths_config_{};
    std::unique_ptr<std::set<std::string>> end_paths_config_{};
    std::map<std::string, std::vector<WorkerConfigInfo>> protoTrigPathLabelMap_{};
    std::vector<WorkerConfigInfo> protoEndPathLabels_{};
  };

} // namespace art

#endif /* art_Framework_Core_PathManager_h */

// Local Variables:
// mode: c++
// End:
