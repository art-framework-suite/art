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

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace art {

  class TriggerResults;

  class TriggerNamesService {

    // Special Member Functions
  public:
    TriggerNamesService(std::vector<std::string> const& triggerPathNames,
                        std::string const& processName,
                        fhicl::ParameterSet const& triggerPSet,
                        fhicl::ParameterSet const& physicsPSet);
    // API
  public:
    // Returns jobPS.process_name
    std::string const& getProcessName() const;
    // Parameter set containing the trigger paths, the key
    // is "trigger_paths", and the value is getTrigPaths().
    fhicl::ParameterSet const& getTriggerPSet() const;
    // Returns trigger path names passed to the ctor.
    std::vector<std::string> const& getTrigPaths() const;
    // Returns count of trigger path names.
    std::size_t size() const;
    // Returns the trigger path name at index i.
    std::string const& getTrigPath(std::size_t const i) const;
    // Returns the trigger bit position of trigger path name,
    // or size() on failure.
    std::size_t findTrigPath(std::string const& name) const;
    // Returns the module names on the named trigger path.
    std::vector<std::string> const& getTrigPathModules(
      std::string const& name) const;
    // Returns the modules names on the trigger path at bit position i.
    std::vector<std::string> const& getTrigPathModules(
      std::size_t const i) const;
    // Returns the module name on the named trigger path at index j.
    std::string const& getTrigPathModule(std::string const& name,
                                         std::size_t const j) const;
    // Returns the module name on the trigger path at bit position i with module
    // index j.
    std::string const& getTrigPathModule(std::size_t const i,
                                         std::size_t const j) const;
    // Implementation details.
  private:
    std::size_t find(std::map<std::string, std::size_t> const& posmap,
                     std::string const& name) const;
    // Data members
  private:
    // Trigger path names, passed to ctor.
    std::vector<std::string> const triggerPathNames_;
    // The art process_name from the top pset.
    std::string const processName_;
    // Parameter set of trigger paths (used by
    // TriggerResults objects), the key is
    // "trigger_paths", and the value is
    // triggerPathNames_, const after ctor.
    fhicl::ParameterSet triggerPSet_{};
    // Maps trigger path name to trigger bit position, const after ctor.
    std::map<std::string, std::size_t> trigPathNameToTrigBitPos_{};
    // Labels of modules on trigger paths, const after ctor.
    std::vector<std::vector<std::string>> moduleNames_{};
  };

} // namespace art

DECLARE_ART_SYSTEM_SERVICE(art::TriggerNamesService, LEGACY)

#endif /* art_Framework_Services_System_TriggerNamesService_h */

// Local Variables:
// mode: c++
// End:
