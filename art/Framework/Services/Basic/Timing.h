#ifndef Services_TIMING_h
#define Services_TIMING_h

//
// Package:     Services
// Class  :     Timing
//


#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/ProvenanceFwd.h"

#include "fhiclcpp/ParameterSet.h"
#include "sigc++/signal.h"


namespace edm {

  struct ActivityRegistry;
  class Event;

  namespace service {
    class Timing
    {
    public:
      Timing(const fhicl::ParameterSet&,ActivityRegistry&);
      ~Timing();

      sigc::signal<void, const ModuleDescription&, double> newMeasurementSignal;
    private:
      void postBeginJob();
      void postEndJob();

      void preEventProcessing(const EventID&, const Timestamp&);
      void postEventProcessing(const Event&);

      void preModule(const ModuleDescription&);
      void postModule(const ModuleDescription&);

      EventID curr_event_;
      double curr_job_; // seconds
      double curr_event_time_;  // seconds
      double curr_module_time_; // seconds
      bool summary_only_;
      bool report_summary_;

      // Min Max and average event times for summary
      //  at end of job
      double max_event_time_;    // seconds
      double min_event_time_;    // seconds
      int total_event_count_;
    };
  }

}  // namespace edm

#endif  // Services_TIMING_h
