#include "art/Utilities/Globals.h"
// vim: set sw=2 expandtab :

#include "fhiclcpp/ParameterSet.h"

#include <string>
#include <vector>

using namespace std;

namespace art {

  Globals::~Globals() = default;
  Globals::Globals() = default;

  Globals*
  Globals::instance()
  {
    static Globals me;
    return &me;
  }

  ScheduleID::size_type
  Globals::nschedules() const
  {
    return nschedules_;
  }

  void
  Globals::setNSchedules(int const nschedules)
  {
    nschedules_ = nschedules;
  }

  ScheduleID::size_type
  Globals::nthreads() const
  {
    return nthreads_;
  }

  void
  Globals::setNThreads(int const nthreads)
  {
    nthreads_ = nthreads;
  }

  string const&
  Globals::processName() const
  {
    return processName_;
  }

  void
  Globals::setProcessName(string const& processName)
  {
    processName_ = processName;
  }

  fhicl::ParameterSet const&
  Globals::triggerPSet() const
  {
    return triggerPSet_;
  }

  void
  Globals::setTriggerPSet(fhicl::ParameterSet const& triggerPSet)
  {
    triggerPSet_ = triggerPSet;
  }

  vector<string> const&
  Globals::triggerPathNames() const
  {
    return triggerPathNames_;
  }

  void
  Globals::setTriggerPathNames(vector<string> const& triggerPathNames)
  {
    triggerPathNames_ = triggerPathNames;
  }

} // namespace art
