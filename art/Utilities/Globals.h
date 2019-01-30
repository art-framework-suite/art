#ifndef art_Utilities_Globals_h
#define art_Utilities_Globals_h
// vim: set sw=2 expandtab :

#include "art/Utilities/ScheduleID.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>
#include <vector>

namespace art {

  class EventProcessor;
  class Scheduler;

  class Globals {
    friend class EventProcessor;
    friend class Scheduler;

  public:
    ~Globals();
    Globals(Globals const&) = delete;
    Globals(Globals&&) = delete;
    Globals& operator=(Globals const&) = delete;
    Globals& operator=(Globals&&) = delete;

    static Globals* instance();
    ScheduleID::size_type nschedules() const;
    ScheduleID::size_type nthreads() const;
    std::string const& processName() const;
    fhicl::ParameterSet const& triggerPSet() const;
    std::vector<std::string> const& triggerPathNames() const;

  private:
    Globals();

    void setNSchedules(int);
    void setNThreads(int);
    void setProcessName(std::string const&);
    void setTriggerPSet(fhicl::ParameterSet const&);
    void setTriggerPathNames(std::vector<std::string> const&);

    int nschedules_{1};
    int nthreads_{1};
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
