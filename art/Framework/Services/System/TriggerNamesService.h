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

#include "art/Framework/Services/Registry/detail/system_service_macros.h"
#include "art/Persistency/Provenance/PathSpec.h"
#include "fhiclcpp/fwd.h"

#include <functional>
#include <string>
#include <vector>

namespace art {
  namespace detail {
    using entry_selector_t = std::function<bool(PathSpec const&)>;
  }

  class TriggerNamesService {
  public:
    TriggerNamesService(std::vector<PathSpec> const& triggerPathSpecs,
                        std::string const& processName,
                        fhicl::ParameterSet const& physicsPSet);
    // Returns jobPS.process_name
    std::string const& getProcessName() const;

    // Returns trigger path names passed to the ctor.
    std::vector<std::string> const& getTrigPaths() const;

    // Returns count of trigger path names.
    std::size_t size() const;

    // Returns the trigger path name for the supplied PathID.
    std::string const& getTrigPath(PathID id) const;

    // Returns the path ID of trigger path name, or an invalid ID on
    // failure.
    PathID findTrigPath(std::string const& name) const;

    // Returns the module names on the named trigger path.
    std::vector<std::string> const& getTrigPathModules(
      std::string const& name) const;
    // Returns the modules names on the trigger path for the supplied pathID.
    std::vector<std::string> const& getTrigPathModules(PathID i) const;
    // Returns the module name on the named trigger path at index j.
    std::string const& getTrigPathModule(std::string const& name,
                                         std::size_t j) const;
    // Returns the module name on the trigger path with the supplied
    // PathID and module index j.
    std::string const& getTrigPathModule(PathID id, std::size_t j) const;

    // Expert only
    size_t index_for(PathID id) const;

  private:
    size_t index_(detail::entry_selector_t selector) const;

    std::vector<PathSpec> triggerPathSpecs_{};
    std::vector<std::string> triggerPathNames_{};
    std::string const processName_;
    // Labels of modules on trigger paths, const after ctor.
    std::vector<std::vector<std::string>> moduleNames_{};
  };

} // namespace art

DECLARE_ART_SYSTEM_SERVICE(art::TriggerNamesService, SHARED)

#endif /* art_Framework_Services_System_TriggerNamesService_h */

// Local Variables:
// mode: c++
// End:
