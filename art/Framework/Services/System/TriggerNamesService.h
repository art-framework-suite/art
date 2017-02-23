#ifndef art_Framework_Services_System_TriggerNamesService_h
#define art_Framework_Services_System_TriggerNamesService_h

// ======================================================================
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
// Multi-threading considerations:
//
//   This service is constructed during process start-up, which is
//   done in a single-threaded environment.  Since all other interface
//   is `const` qualified, and there are no mutables, reading from
//   this service is inherently thread-safe.
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include <map>
#include <string>
#include <vector>

namespace art {
  class TriggerResults;
  class TriggerNamesService;
}

// ----------------------------------------------------------------------

class art::TriggerNamesService {
public:

  using Strings = std::vector<std::string>;
  using PosMap = std::map<std::string, unsigned int>;
  using size_type = PosMap::size_type;

  TriggerNamesService(fhicl::ParameterSet const& procPS,
                      std::vector<std::string> const& triggerPathNames);

  // Trigger path information for the current process
  size_type size() const { return trignames_.size(); }
  Strings const& getTrigPaths() const { return trignames_; }
  std::string const& getTrigPath(size_type const i) const { return trignames_.at(i);}
  size_type findTrigPath(std::string const& name) const { return find(trigpos_, name);}

  // Get the ordered vector of trigger names that corresponds to the
  // bits in the TriggerResults object.  Unlike the other functions in
  // this class, the next two functions will retrieve the names for
  // previous processes.  If the TriggerResults object is from the
  // current process, this only works for modules in end paths,
  // because the TriggerResults object is not created until the normal
  // paths complete execution.  Returns false if it fails to find the
  // trigger path names.
  bool getTrigPaths(TriggerResults const& triggerResults, Strings& trigPaths) const;

  auto getTrigPathModules(std::string const& name) const -> Strings const&;
  auto getTrigPathModules(size_type const i) const -> Strings const&;
  std::string const& getTrigPathModule(std::string const& name, size_type const j) const;
  std::string const& getTrigPathModule(size_type const i, size_type const j) const;

  size_type find(PosMap const& posmap, std::string const& name) const
  {
    auto const pos = posmap.find(name);
    if (pos == posmap.cend()) {
      return posmap.size();
    }
    else {
      return pos->second;
    }
  }

  std::string const& getProcessName() const { return process_name_; }
  bool wantSummary() const { return wantSummary_; }

  // Parameter set containing the trigger paths
  fhicl::ParameterSet const& getTriggerPSet() const { return trigger_pset_; }

private:

  Strings trignames_;
  PosMap  trigpos_ {};
  Strings end_names_ {};
  PosMap  end_pos_ {};

  fhicl::ParameterSet trigger_pset_ {}; // Parameter set of trigger paths
                                        // (used by TriggerResults
                                        // objects).
  std::vector<Strings> modulenames_ {}; // Labels of modules on trigger paths

  std::string process_name_;
  bool wantSummary_;
};  // TriggerNamesService

// ======================================================================
// Implementation
inline
auto art::TriggerNamesService::getTrigPathModules(std::string const& name) const
  -> Strings const&
{
  return modulenames_.at(find(trigpos_, name));
}

inline
auto art::TriggerNamesService::getTrigPathModules(size_type const i) const
  -> Strings const&
{
  return modulenames_.at(i);
}

inline
std::string const&
art::TriggerNamesService::getTrigPathModule(std::string const& name, size_type const j) const
{
  return modulenames_.at(find(trigpos_, name)).at(j);
}

inline
std::string const&
art::TriggerNamesService::getTrigPathModule(size_type const i, size_type const j) const
{
  return modulenames_.at(i).at(j);
}

// ======================================================================

DECLARE_ART_SYSTEM_SERVICE(art::TriggerNamesService, LEGACY)
#endif /* art_Framework_Services_System_TriggerNamesService_h */

// Local Variables:
// mode: c++
// End:
