// ======================================================================
//
// Timing
//
// ======================================================================

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/ProvenanceFwd.h"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <algorithm>
#include <limits>
#include <sys/time.h>

namespace art {
  class Timing;
}

using art::Timing;
using fhicl::ParameterSet;

// ======================================================================

class art::Timing
{
public:
  Timing(ParameterSet const&, ActivityRegistry&);

private:
  void postBeginJob();
  void postEndJob();

  void preEventProcessing(Event const&);
  void postEventProcessing(Event const&);

  void preModule(ModuleDescription const&);
  void postModule(ModuleDescription const&);

  EventID curr_event_;
  double curr_job_;         // seconds
  double curr_event_time_;  // seconds
  double curr_module_time_; // seconds
  bool summary_only_;

  // Min Max and average event times for summary at end of job
  double max_event_time_;    // seconds
  double min_event_time_;    // seconds
  double avg_event_time_;    // seconds
  double nEvt_;

};  // Timing

// ======================================================================

static double getTime()
{
  struct timeval t;
  if(gettimeofday(&t, 0) < 0)
    throw cet::exception("SysCallFailed", "Failed call to gettimeofday");

  return (double)t.tv_sec + (double(t.tv_usec) * 1E-6);
}

// ----------------------------------------------------------------------

Timing::Timing(ParameterSet const& iPS, ActivityRegistry& iRegistry)
  : summary_only_(iPS.get<bool>("summaryOnly", false))
  , max_event_time_( std::numeric_limits<double>::lowest() )
  , min_event_time_( std::numeric_limits<double>::max()    )
  , avg_event_time_( 0. )
  , nEvt_(0.)
{
  iRegistry.sPostBeginJob.watch(this, &Timing::postBeginJob);
  iRegistry.sPostEndJob.watch(this, &Timing::postEndJob);

  iRegistry.sPreProcessEvent.watch(this, &Timing::preEventProcessing);
  iRegistry.sPostProcessEvent.watch(this, &Timing::postEventProcessing);

  iRegistry.sPreModule.watch(this, &Timing::preModule);
  iRegistry.sPostModule.watch(this, &Timing::postModule);
}

// ----------------------------------------------------------------------

void Timing::postBeginJob()
{
  if (not summary_only_) {
    mf::LogAbsolute("TimeReport")
      << "TimeReport> Report activated\n"
         "TimeReport> Report columns headings for events: "
         "eventnum runnum timetaken\n"
         "TimeReport> Report columns headings for modules: "
         "eventnum runnum modulelabel modulename timetaken";
  }
  curr_job_ = getTime();
}

void Timing::postEndJob()
{
  double const t = getTime() - curr_job_;

  mf::LogAbsolute("TimeReport")
    << "TimeReport> Time report complete in "
    << t << " seconds\n"
    << " Time Summary: \n"
    << " Min: " << min_event_time_ << "\n"
    << " Max: " << max_event_time_ << "\n"
    << " Avg: " << avg_event_time_ << "\n";

}

// ----------------------------------------------------------------------

void Timing::preEventProcessing(Event const& ev)
{
  curr_event_ = ev.id();
  curr_event_time_ = getTime();
}

void Timing::postEventProcessing(Event const&)
{

  double const t = getTime() - curr_event_time_;

  if (not summary_only_) {
    mf::LogAbsolute("TimeEvent")
       << "TimeEvent> "
       << curr_event_ << " " << t;
  }

  max_event_time_ = std::max( max_event_time_, t );
  min_event_time_ = std::min( min_event_time_, t );
  avg_event_time_ = (nEvt_*avg_event_time_ + t)/(nEvt_+1);

  ++nEvt_;

}

// ----------------------------------------------------------------------

void Timing::preModule(ModuleDescription const&)
{
  curr_module_time_ = getTime();
}

void Timing::postModule(ModuleDescription const& desc)
{
  double const t = getTime() - curr_module_time_;
  if (not summary_only_) {
    mf::LogAbsolute("TimeModule")
      << "TimeModule> "
      << curr_event_ << " "
      << desc.moduleLabel() << " "
      << desc.moduleName()<< " "
      << t;
  }
}

// ======================================================================

// The DECLARE macro call should be moved to the header file, should you
// create one.
DECLARE_ART_SERVICE(Timing, LEGACY)
DEFINE_ART_SERVICE(Timing)

// ======================================================================
