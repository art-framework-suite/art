#ifndef art_Utilities_Globals_h
#define art_Utilities_Globals_h
// vim: set sw=2 expandtab :

#include "art/Utilities/ScheduleID.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Utilities/TypeID.h"
#include "fhiclcpp/ParameterSet.h"
#include "hep_concurrency/RecursiveMutex.h"

#include <array>
#include <atomic>
#include <cstdlib>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace art {

  class EventProcessor;
  class Scheduler;

  class Globals {

    friend class EventProcessor;
    friend class Scheduler;

    // MEMBER FUNCTIONS -- Special Member Functions
  public:
    ~Globals();
    Globals(Globals const&) = delete;
    Globals(Globals&&) = delete;
    Globals& operator=(Globals const&) = delete;
    Globals& operator=(Globals&&) = delete;

    // MEMBER FUNCTIONS -- Special Member Functions
  private:
    Globals();

    // MEMBER FUNCTIONS -- Static API
  public:
    static Globals* instance();

    // MEMBER FUNCTIONS -- API for getting system-wide settings
  public:
    ScheduleID::size_type nschedules() const;
    ScheduleID::size_type nthreads() const;
    std::string const& processName() const;
    fhicl::ParameterSet const& triggerPSet() const;
    std::vector<std::string> const& triggerPathNames() const;

    // MEMBER FUNCTIONS -- API for setting system-wide settings, only for
    // friends
  private:
    void setNSchedules(int);
    void setNThreads(int);
    void setProcessName(std::string const&);
    void setTriggerPSet(fhicl::ParameterSet const&);
    void setTriggerPathNames(std::vector<std::string> const&);

    // MEMBER DATA
  private:
    // The services.scheduler.nschedules parameter.
    int nschedules_{1};

    // The services.scheduler.nthreads parameter.
    int nthreads_{1};

    // The art process_name from the job pset.
    std::string processName_;

    // Parameter set of trigger paths, the key is "trigger_paths",
    // and the value is triggerPathNames_.
    fhicl::ParameterSet triggerPSet_;

    // Trigger path names, passed to ctor.
    std::vector<std::string> triggerPathNames_;
  };

} // namespace art

  // Local Variables:
  // mode: c++
  // End:

#endif /* art_Utilities_Globals_h */
