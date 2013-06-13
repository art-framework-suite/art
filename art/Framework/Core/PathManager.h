#ifndef art_Framework_Core_PathManager_h
#define art_Framework_Core_PathManager_h
////////////////////////////////////////////////////////////////////////
// PathManager.
//
// Class to handle the processing of the configuration of modules in
// art, including the creation of paths and construction of modules as
// appropriate.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/Path.h"
#include "art/Framework/Core/detail/ModuleFactory.h"
#include "art/Framework/Core/detail/ModuleConfigInfo.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace art {
  class PathManager;
}

class art::PathManager {
public:
  typedef std::vector<Worker *> Workers;
  typedef std::vector<std::string> vstring;

  PathManager(fhicl::ParameterSet const & procPS,
              MasterProductRegistry & preg,
              ActionTable & exceptActions,
              std::shared_ptr<ActivityRegistry> const & areg);

  vstring const & triggerPathNames() const;

private:
  typedef std::vector<Path> Paths;
  typedef std::map<std::string, std::shared_ptr<Worker>> WorkerMap;

  detail::ModuleConfigInfoMap fillAllModules_();
  vstring processPathConfigs_();
  // Returns true if path is a trigger path.
  bool processOnePathConfig_(std::string const & path_name,
                             fhicl::ParameterSet const & path_pset);

  fhicl::ParameterSet procPS_;
  MasterProductRegistry & preg_;
  ActionTable & exceptActions_;
  std::shared_ptr<ActivityRegistry> const & areg_;

  detail::ModuleFactory fact_;
  detail::ModuleConfigInfoMap allModules_;
  WorkerMap endPathWorkers_;
  std::unique_ptr<Path> endPath_;
  std::vector<WorkerMap> triggerPathWorkers_; // Per-schedule.
  std::vector<Paths> triggerPaths_; // Per-schedule.
  vstring triggerPathNames_;
};

#endif /* art_Framework_Core_PathManager_h */

// Local Variables:
// mode: c++
// End:
