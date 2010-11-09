//
// Package:     Services
// Class  :     Timing
//


#include "art/Framework/Services/Basic/Timing.h"

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/Service.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "cetlib/exception.h"

#include "MessageFacility/MessageLogger.h"
#include "fhiclcpp/ParameterSet.h"
  using fhicl::ParameterSet;

#include <sys/time.h>

#include <iostream>


namespace art {

  namespace service {

    static double getTime()
    {
      struct timeval t;
      if(gettimeofday(&t,0)<0)
        throw cet::exception("SysCallFailed","Failed call to gettimeofday");

      return (double)t.tv_sec + (double(t.tv_usec) * 1E-6);
    }

    Timing::Timing(const ParameterSet& iPS, ActivityRegistry&iRegistry):
      summary_only_(iPS.get<bool>("summaryOnly",false)),
      report_summary_(iPS.get<bool>("useJobReport",true)),
      max_event_time_(0.),
      min_event_time_(0.),
      total_event_count_(0)

    {
      iRegistry.watchPostBeginJob(this,&Timing::postBeginJob);
      iRegistry.watchPostEndJob(this,&Timing::postEndJob);

      iRegistry.watchPreProcessEvent(this,&Timing::preEventProcessing);
      iRegistry.watchPostProcessEvent(this,&Timing::postEventProcessing);

      iRegistry.watchPreModule(this,&Timing::preModule);
      iRegistry.watchPostModule(this,&Timing::postModule);
    }


    Timing::~Timing()
    {
    }

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
      double t = getTime() - curr_job_;
      double average_event_t = t / total_event_count_;
      mf::LogAbsolute("TimeReport")                            // Changelog 1
        << "TimeReport> Time report complete in "
        << t << " seconds\n"
        << " Time Summary: \n"
        << " Min: " << min_event_time_ << "\n"
        << " Max: " << max_event_time_ << "\n"
        << " Avg: " << average_event_t << "\n";
      if (report_summary_){
        std::map<std::string, double> reportData;

        reportData.insert(std::make_pair("MinEventTime", min_event_time_));
        reportData.insert(std::make_pair("MaxEventTime", max_event_time_));
        reportData.insert(std::make_pair("AvgEventTime", average_event_t));
        reportData.insert(std::make_pair("TotalTime", t));
      }

    }

    void Timing::preEventProcessing(const art::EventID& iID,
                                    const art::Timestamp& iTime)
    {
      curr_event_ = iID;
      curr_event_time_ = getTime();


    }
    void Timing::postEventProcessing(const Event& e)
    {
      double t = getTime() - curr_event_time_;
      if (not summary_only_) {
        mf::LogAbsolute("TimeEvent") << "TimeEvent> "
          << curr_event_.event() << " " << curr_event_.run() << " " << t;
      }
      if (total_event_count_ == 0) {
        max_event_time_ = t;
        min_event_time_ = t;
      }

      if (t > max_event_time_) max_event_time_ = t;
      if (t < min_event_time_) min_event_time_ = t;
      total_event_count_ = total_event_count_ + 1;
    }

    void Timing::preModule(const ModuleDescription&)
    {
      curr_module_time_ = getTime();
    }

    void Timing::postModule(const ModuleDescription& desc)
    {
      double t = getTime() - curr_module_time_;
      if (not summary_only_) {
        mf::LogAbsolute("TimeModule") << "TimeModule> "
           << curr_event_.event() << " "
           << curr_event_.run() << " "
           << desc.moduleLabel_ << " "
           << desc.moduleName_ << " "
           << t;
      }

      newMeasurementSignal(desc,t);
    }

  }

}  // namespace art
