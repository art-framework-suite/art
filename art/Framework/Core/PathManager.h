#ifndef art_Framework_Core_PathManager_h
#define art_Framework_Core_PathManager_h
////////////////////////////////////////////////////////////////////////
// PathManager.
//
// Class to handle the processing of the configuration of modules in
// art, including the creation of paths and construction of modules as
// appropriate.
//
// Intended to be constructed early, prior to services, since
// TriggerNamesService will need some of the information herein at
// construction time.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/Path.h"
#include "art/Framework/Core/PathsInfo.h"
#include "art/Framework/Core/detail/ModuleConfigInfo.h"
#include "art/Framework/Core/detail/ModuleFactory.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "canvas/Persistency/Common/HLTGlobalStatus.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Utilities/ScheduleID.h"
#include "fhiclcpp/ParameterSet.h"

#include <iosfwd>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace art {
  class PathManager;
}

class art::PathManager {
public:
  PathManager(PathManager const&) = delete;
  PathManager& operator=(PathManager const&) = delete;

  using Workers = std::vector<Worker*>;
  using vstring = std::vector<std::string>;

  PathManager(fhicl::ParameterSet const& procPS,
              MasterProductRegistry& preg,
              ActionTable& exceptActions,
              ActivityRegistry& areg);

  vstring const& triggerPathNames() const;

  // These methods may trigger module construction.
  PathsInfo& endPathInfo();
  PathsInfo& triggerPathsInfo(ScheduleID sID);

  void resetAll(); // Reset trigger results ready for next event.

private:

  using ModInfos = PathsInfo::ModInfos;

  detail::ModuleConfigInfoMap fillAllModules_();
  vstring processPathConfigs_();
  // Returns true if path is an end path.
  bool processOnePathConfig_(std::string const& path_name,
                             vstring const& path_seq,
                             vstring& trigger_path_names,
                             std::ostream& error_stream);

  fhicl::ParameterSet procPS_;
  MasterProductRegistry& preg_;
  ActionTable& exceptActions_;
  ActivityRegistry& areg_;

  // Cached parameters.

  // Backwards compatibility cached parameters.
  std::unique_ptr<std::set<std::string>> trigger_paths_config_;
  std::unique_ptr<std::set<std::string>> end_paths_config_;

  detail::ModuleFactory fact_{};
  detail::ModuleConfigInfoMap allModules_;
  std::map<std::string, ModInfos> protoTrigPathMap_{};
  ModInfos protoEndPathInfo_{};
  vstring triggerPathNames_;
  PathsInfo endPathInfo_;
  std::map<ScheduleID, PathsInfo> triggerPathsInfo_{}; // Per-schedule.
};

inline
art::PathManager::vstring const&
art::PathManager::triggerPathNames() const
{
  return triggerPathNames_;
}

#endif /* art_Framework_Core_PathManager_h */

// Local Variables:
// mode: c++
// End:
