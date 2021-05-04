#ifndef art_Framework_Services_System_TriggerNamesService_h
#define art_Framework_Services_System_TriggerNamesService_h
// vim: set sw=2 expandtab :

//
// TriggerNamesService
//
// This service makes the trigger names available.  They are provided in
// the same order that the pass/fail status of these triggers is recorded
// in the TriggerResults object.  These trigger names are the names of
// the paths that appear in the configuration (excluding end paths).  The
// order is the same as in the configuration.
//
// There are also accessors for the end path names.
//
// There are other accessors for other trigger related information from
// the job configuration: the process name, whether a report on trigger
// results was requested and the parameter set containing the list of
// trigger paths.
//
// Almost all the functions return information related to the current
// process only.  The second getTrigPaths function is an exception.
// It will return the trigger path names from previous processes.
//

#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/Registry/detail/system_service_macros.h"
#include "art/Persistency/Provenance/PathSpec.h"
#include "canvas/Persistency/Common/HLTPathStatus.h"
#include "canvas/Persistency/Common/TriggerResults.h"
#include "fhiclcpp/fwd.h"

#include <functional>
#include <string>
#include <vector>

namespace art {
  class ActivityRegistry;

  namespace detail {
    using entry_selector_t = std::function<bool(PathSpec const&)>;
  }

  class TriggerNamesService {
  public:
    TriggerNamesService(fhicl::ParameterSet const& trigger_paths_pset,
                        fhicl::ParameterSet const& physics_pset);

    // For all processes
    TriggerResults const& triggerResults(
      Event const& e,
      std::string const& process_name = "current_process") const;

    std::map<std::string, HLTPathStatus> pathResults(
      Event const& e,
      std::string const& process_name = "current_process") const;

    // Current process only
    std::string const& getProcessName() const;
    std::vector<std::string> const& getTrigPaths() const;
    std::string const& getTrigPath(PathID const id) const;
    PathID findTrigPath(std::string const& name) const;
    std::vector<std::string> const& getTrigPathModules(
      std::string const& name) const;
    std::vector<std::string> const& getTrigPathModules(PathID id) const;

    //  - Expert only
    size_t index_for(PathID id) const;

    struct DataPerProcess {
      std::vector<PathSpec> triggerPathSpecs{};
      std::vector<std::string> triggerPathNames{};
      std::vector<std::vector<std::string>> moduleNames{};
    };

  private:
    size_t index_(detail::entry_selector_t selector) const;
    DataPerProcess const& currentData_() const;

    std::map<std::string, DataPerProcess> mutable dataPerProcess_;
  };

} // namespace art

DECLARE_ART_SYSTEM_SERVICE(art::TriggerNamesService, SHARED)

#endif /* art_Framework_Services_System_TriggerNamesService_h */

// Local Variables:
// mode: c++
// End:
