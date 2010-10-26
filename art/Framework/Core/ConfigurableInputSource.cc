/*----------------------------------------------------------------------

----------------------------------------------------------------------*/


#include "art/Framework/Core/ConfigurableInputSource.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/Core/Run.h"
#include "art/Framework/Core/RunPrincipal.h"
#include "art/Framework/Core/SubRun.h"
#include "art/Framework/Core/SubRunPrincipal.h"
#include "art/Persistency/Provenance/RunAuxiliary.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"

#include "fhiclcpp/ParameterSet.h"


namespace art {
  //used for defaults
  static const unsigned int kNanoSecPerSec = 1000000000U;
  static const unsigned int kAveEventPerSec = 200U;

  ConfigurableInputSource::ConfigurableInputSource(fhicl::ParameterSet const& pset,
                                       InputSourceDescription const& desc, bool realData) :
    InputSource(pset, desc),
    numberEventsInRun_(pset.get<unsigned int>("numberEventsInRun", remainingEvents())),
    numberEventsInSubRun_(pset.get<unsigned int>("numberEventsInSubRun", remainingEvents())),
    presentTime_(pset.get<unsigned int>("firstTime", 0)),  //time in ns
    origTime_(presentTime_),
    timeBetweenEvents_(pset.get<unsigned int>("timeBetweenEvents", kNanoSecPerSec/kAveEventPerSec)),
    eventCreationDelay_(pset.get<unsigned int>("eventCreationDelay", 0)),
    numberEventsInThisRun_(0),
    numberEventsInThisSubRun_(0),
    zerothEvent_(pset.get<unsigned int>("firstEvent", 1) - 1),
    eventID_(pset.get<unsigned int>("firstRun", 1), zerothEvent_),
    origEventID_(eventID_),
    subRun_(pset.get<unsigned int>("firstSubRun", 1)),
    origSubRunNumber_t_(subRun_),
    newRun_(true),
    newSubRun_(true),
    subRunSet_(false),
    eventSet_(false),
    ep_(0),
    isRealData_(realData),
    eType_(EventAuxiliary::Any)
  {
    setTimestamp(Timestamp(presentTime_));
    // We need to map this string to the EventAuxiliary::ExperimentType enumeration
    // std::string eType = pset.get<std::string>("experimentType", std::string("Any"))),
  }

  ConfigurableInputSource::~ConfigurableInputSource() {
  }

  boost::shared_ptr<RunPrincipal>
  ConfigurableInputSource::readRun_() {
    Timestamp ts = Timestamp(presentTime_);
    RunAuxiliary runAux(eventID_.run(), ts, Timestamp::invalidTimestamp());
    boost::shared_ptr<RunPrincipal> runPrincipal(
        new RunPrincipal(runAux, productRegistry(), processConfiguration()));
    RunPrincipal & rp =
       const_cast<RunPrincipal &>(*runPrincipal);
    Run run(rp, moduleDescription());
    beginRun(run);
    commitRun(run);
    newRun_ = false;
    return runPrincipal;
  }

  boost::shared_ptr<SubRunPrincipal>
  ConfigurableInputSource::readSubRun_() {
    if (processingMode() == Runs) return boost::shared_ptr<SubRunPrincipal>();
    Timestamp ts = Timestamp(presentTime_);
    SubRunAuxiliary subRunAux(runPrincipal()->run(), subRun_, ts, Timestamp::invalidTimestamp());
    boost::shared_ptr<SubRunPrincipal> subRunPrincipal(
        new SubRunPrincipal(
            subRunAux, productRegistry(), processConfiguration()));
    SubRun lb(*subRunPrincipal, moduleDescription());
    beginSubRun(lb);
    commitSubRun(lb);
    newSubRun_ = false;
    return subRunPrincipal;
  }

  std::auto_ptr<EventPrincipal>
  ConfigurableInputSource::readEvent_() {
    assert(ep_.get() != 0 || processingMode() != RunsSubRunsAndEvents);
    return ep_;
  }

  void
  ConfigurableInputSource::reallyReadEvent(SubRunNumber_t subRun) {
    if (processingMode() != RunsSubRunsAndEvents) return;
    EventSourceSentry sentry(*this);
    EventAuxiliary eventAux(eventID_,
      processGUID(), Timestamp(presentTime_), subRun, isRealData_, eType_);
    std::auto_ptr<EventPrincipal> result(
        new EventPrincipal(eventAux, productRegistry(), processConfiguration()));
    Event e(*result, moduleDescription());
    if (!produce(e)) {
      ep_.reset();
      return;
    }
    commitEvent(e);
    ep_ = result;
  }

  void
  ConfigurableInputSource::skip(int offset) {
    for (; offset < 0; ++offset) {
       eventID_ = eventID_.previous();
    }
    for (; offset > 0; --offset) {
       eventID_ = eventID_.next();
    }
  }


  void
  ConfigurableInputSource::setRun(RunNumber_t r) {
    // No need to check for invalid (zero) run number,
    // as this is a legitimate way of stopping the job.
    // Do nothing if the run is not changed.
    if (r != eventID_.run()) {
      eventID_ = EventID(r, zerothEvent_);
      subRun_ = origSubRunNumber_t_;
      numberEventsInThisRun_ = 0;
      numberEventsInThisSubRun_ = 0;
      newRun_ = newSubRun_ = true;
      resetSubRunPrincipal();
      resetRunPrincipal();
    }
  }

  void
  ConfigurableInputSource::beginRun(Run&)
  { }

  void
  ConfigurableInputSource::endRun(Run&)
  { }

  void
  ConfigurableInputSource::beginSubRun(SubRun &)
  { }

  void
  ConfigurableInputSource::endSubRun(SubRun &)
  { }

  void
  ConfigurableInputSource::setSubRun(SubRunNumber_t lb) {
    // Protect against invalid subRun.
    if (lb == SubRunNumber_t()) {
        lb = origSubRunNumber_t_;
    }
    // Do nothing if the subRun is not changed.
    if (lb != subRun_) {
      subRun_ = lb;
      numberEventsInThisSubRun_ = 0;
      newSubRun_ = true;
      resetSubRunPrincipal();
    }
    subRunSet_ = true;
  }

  void
  ConfigurableInputSource::rewind_() {
    subRun_ = origSubRunNumber_t_;
    presentTime_ = origTime_;
    eventID_ = origEventID_;
    numberEventsInThisRun_ = 0;
    numberEventsInThisSubRun_ = 0;
    newRun_ = newSubRun_ = true;
    resetSubRunPrincipal();
    resetRunPrincipal();
  }

  InputSource::ItemType
  ConfigurableInputSource::getNextItemType() {
    if (newRun_) {
      if (eventID_.run() == RunNumber_t()) {
        ep_.reset();
        return IsStop;
      }
      return IsRun;
    }
    if (newSubRun_) {
      return IsSubRun;
    }
    if(ep_.get() != 0) return IsEvent;
    EventID oldEventID = eventID_;
    SubRunNumber_t oldSubRun = subRun_;
    if (!eventSet_) {
      subRunSet_ = false;
      setRunAndEventInfo();
      eventSet_ = true;
    }
    if (eventID_.run() == RunNumber_t()) {
      ep_.reset();
      return IsStop;
    }
    if (oldEventID.run() != eventID_.run()) {
      //  New Run
      // reset these since this event is in the new run
      numberEventsInThisRun_ = 0;
      numberEventsInThisSubRun_ = 0;
      // If the user did not explicitly set the subRun number,
      // reset it back to the beginning.
      if (!subRunSet_) {
        subRun_ = origSubRunNumber_t_;
      }
      newRun_ = newSubRun_ = true;
      resetSubRunPrincipal();
      resetRunPrincipal();
      return IsRun;
    }
      // Same Run
    if (oldSubRun != subRun_) {
      // New Subrun
      numberEventsInThisSubRun_ = 0;
      newSubRun_ = true;
      resetSubRunPrincipal();
      if (processingMode() != Runs) {
        return IsSubRun;
      }
    }
    ++numberEventsInThisRun_;
    ++numberEventsInThisSubRun_;
    reallyReadEvent(subRun_);
    if (ep_.get() == 0) {
      return IsStop;
    }
    eventSet_ = false;
    return IsEvent;
  }

  void
  ConfigurableInputSource::setRunAndEventInfo() {
    //NOTE: numberEventsInRun < 0 means go forever in this run
    if (numberEventsInRun_ < 1 || numberEventsInThisRun_ < numberEventsInRun_) {
      // same run
      eventID_ = eventID_.next();
      if (!(numberEventsInSubRun_ < 1 || numberEventsInThisSubRun_ < numberEventsInSubRun_)) {
        // new subrun
        ++subRun_;
      }
    } else {
      // new run
      eventID_ = eventID_.nextRunFirstEvent();
    }
    presentTime_ += timeBetweenEvents_;
    if (eventCreationDelay_ > 0) {usleep(eventCreationDelay_);}
  }

}
