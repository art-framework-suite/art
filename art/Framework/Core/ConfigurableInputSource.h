#ifndef Framework_ConfigurableInputSource_h
#define Framework_ConfigurableInputSource_h

// ======================================================================
//
// ConfigurableInputSource
//
// ======================================================================

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Persistency/Provenance/EventAuxiliary.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "art/Persistency/Provenance/Timestamp.h"
#include "boost/shared_ptr.hpp"
#include "fhiclcpp/ParameterSet.h"

// ----------------------------------------------------------------------

namespace art {

  class ConfigurableInputSource
    : public InputSource
  {
  public:
    explicit ConfigurableInputSource(fhicl::ParameterSet const& pset,
                                     InputSourceDescription const& desc,
                                     bool realData = true);
    virtual ~ConfigurableInputSource();

    unsigned int numberEventsInRun() const {return numberEventsInRun_;}
    unsigned int numberEventsInSubRun() const {return numberEventsInSubRun_;}
    TimeValue_t presentTime() const {return presentTime_;}
    unsigned int timeBetweenEvents() const {return timeBetweenEvents_;}
    unsigned int eventCreationDelay() const {return eventCreationDelay_;}
    unsigned int numberEventsInThisRun() const {return numberEventsInThisRun_;}
    unsigned int numberEventsInThisSubRun() const {return numberEventsInThisSubRun_;}
    RunNumber_t run() const {return eventID_.run();}
    EventNumber_t event() const {return eventID_.event();}
    SubRunNumber_t subRun() const {return subRun_;}

  protected:
    void setEventNumber(EventNumber_t e) {
      RunNumber_t r = run();
      eventID_ = EventID(r, e);
      eventSet_ = true;
    }
    void setTime(TimeValue_t t) {presentTime_ = t;}
    void reallyReadEvent(SubRunNumber_t subRun);

  private:
    virtual ItemType getNextItemType();
    virtual void setRunAndEventInfo();
    virtual bool produce(Event & e) = 0;
    virtual void beginRun(Run &);
    virtual void endRun(Run &);
    virtual void beginSubRun(SubRun &);
    virtual void endSubRun(SubRun &);
    virtual std::auto_ptr<EventPrincipal> readEvent_();
    virtual boost::shared_ptr<SubRunPrincipal> readSubRun_();
    virtual boost::shared_ptr<RunPrincipal> readRun_();
    virtual void skip(int offset);
    virtual void setRun(RunNumber_t r);
    virtual void setSubRun(SubRunNumber_t lb);
    virtual void rewind_();

    unsigned int numberEventsInRun_;
    unsigned int numberEventsInSubRun_;
    TimeValue_t presentTime_;
    TimeValue_t origTime_;
    unsigned int timeBetweenEvents_;
    unsigned int eventCreationDelay_;  /* microseconds */

    unsigned int numberEventsInThisRun_;
    unsigned int numberEventsInThisSubRun_;
    unsigned int const zerothEvent_;
    EventID eventID_;
    EventID origEventID_;
    SubRunNumber_t subRun_;
    SubRunNumber_t origSubRunNumber_t_;
    bool newRun_;
    bool newSubRun_;
    bool subRunSet_;
    bool eventSet_;
    std::auto_ptr<EventPrincipal> ep_;
    bool isRealData_;
    EventAuxiliary::ExperimentType eType_;
  };  // ConfigurableInputSource

}  // art

// ======================================================================

#endif
