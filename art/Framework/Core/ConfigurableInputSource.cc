// ======================================================================
//
// ConfigurableInputSource
//
// ======================================================================

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

using namespace art;

typedef  unsigned int  uint;

  //used for defaults
  static const uint kNanoSecPerSec = 1000000000U;
  static const uint kAveEventPerSec = 200U;

// ----------------------------------------------------------------------

ConfigurableInputSource::ConfigurableInputSource
  ( fhicl::ParameterSet const& pset
  , InputSourceDescription const& desc
  , bool realData
  )
: InputSource              ( pset, desc )
, numberEventsInRun_       ( pset.get<uint>("numberEventsInRun", remainingEvents()) )
, numberEventsInSubRun_    ( pset.get<uint>("numberEventsInSubRun", remainingEvents()) )
, presentTime_             ( pset.get<uint>("firstTime", 0u) )  //time in ns
, origTime_                ( presentTime_ )
, timeBetweenEvents_       ( pset.get<uint>("timeBetweenEvents", kNanoSecPerSec/kAveEventPerSec) )
, eventCreationDelay_      ( pset.get<uint>("eventCreationDelay", 0u) )
, numberEventsInThisRun_   ( 0 )
, numberEventsInThisSubRun_( 0 )
, eventID_                 ( )
, origEventID_             ( ) // In body.
, newRun_                  ( true )
, newSubRun_               ( true )
, subRunSet_               ( false )
, eventSet_                ( false )
, ep_                      ( 0 )
, isRealData_              ( realData )
, eType_                   ( EventAuxiliary::Any)
{
  setTimestamp(Timestamp(presentTime_));
  // We need to map this string to the EventAuxiliary::ExperimentType enumeration
  // std::string eType = pset.get<std::string>("experimentType", std::string("Any"))),

  RunNumber_t firstRun;
  bool haveFirstRun = pset.get_if_present("firstRun", firstRun);
  SubRunNumber_t firstSubRun;
  bool haveFirstSubRun = pset.get_if_present("firstSubRun", firstRun);
  EventNumber_t firstEvent;
  bool haveFirstEvent = pset.get_if_present("firstEvent", firstEvent);
  RunID firstRunID = haveFirstRun?RunID(firstRun):RunID::firstRun();
  SubRunID firstSubRunID = haveFirstSubRun?SubRunID(firstRunID.run(), firstSubRun):
     SubRunID::firstSubRun(firstRunID);
  origEventID_ = haveFirstEvent?EventID(firstSubRunID.run(),
                                        firstSubRunID.subRun(),
                                        firstEvent):
     EventID::firstEvent(firstSubRunID);
  eventID_ = origEventID_;
}

ConfigurableInputSource::~ConfigurableInputSource()
{ }

boost::shared_ptr<RunPrincipal>
  ConfigurableInputSource::readRun_()
{
  Timestamp ts = Timestamp(presentTime_);
  RunAuxiliary runAux(eventID_.runID(), ts, Timestamp::invalidTimestamp());
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
  ConfigurableInputSource::readSubRun_()
{
  if (processingMode() == Runs) return boost::shared_ptr<SubRunPrincipal>();
  Timestamp ts = Timestamp(presentTime_);
  SubRunAuxiliary subRunAux(eventID_.subRunID(), ts, Timestamp::invalidTimestamp());
  boost::shared_ptr<SubRunPrincipal> subRunPrincipal(
      new SubRunPrincipal(
          subRunAux, productRegistry(), processConfiguration()));
  SubRun sr(*subRunPrincipal, moduleDescription());
  beginSubRun(sr);
  commitSubRun(sr);
  newSubRun_ = false;
  return subRunPrincipal;
}

std::auto_ptr<EventPrincipal>
  ConfigurableInputSource::readEvent_()
{
  assert(ep_.get() != 0 || processingMode() != RunsSubRunsAndEvents);
  return ep_;
}

void
ConfigurableInputSource::reallyReadEvent() {
  if (processingMode() != RunsSubRunsAndEvents) return;
  EventSourceSentry sentry(*this);
  EventAuxiliary eventAux(eventID_,
                          processGUID(),
                          Timestamp(presentTime_),
                          isRealData_,
                          eType_);
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
  ConfigurableInputSource::skip(int offset)
{
  for (; offset < 0; ++offset) {
     eventID_ = eventID_.previous();
  }
  for (; offset > 0; --offset) {
     eventID_ = eventID_.next();
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
  ConfigurableInputSource::rewind_()
{
  presentTime_ = origTime_;
  eventID_ = origEventID_;
  numberEventsInThisRun_ = 0;
  numberEventsInThisSubRun_ = 0;
  newRun_ = newSubRun_ = true;
  resetSubRunPrincipal();
  resetRunPrincipal();
}

InputSource::ItemType
  ConfigurableInputSource::getNextItemType()
{
  if (newRun_) {
     if (!eventID_.runID().isValid() ) {
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
  if (!eventSet_) {
    subRunSet_ = false;
    setRunAndEventInfo();
    eventSet_ = true;
  }
  if (!eventID_.runID().isValid()) {
    ep_.reset();
    return IsStop;
  }
  if (oldEventID.runID() != eventID_.runID()) {
    //  New Run
    // reset these since this event is in the new run
    numberEventsInThisRun_ = 0;
    numberEventsInThisSubRun_ = 0;
    newRun_ = newSubRun_ = true;
    resetSubRunPrincipal();
    resetRunPrincipal();
    return IsRun;
  }
    // Same Run
  if (oldEventID.subRunID() != eventID_.subRunID()) {
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
  reallyReadEvent();
  if (ep_.get() == 0) {
    return IsStop;
  }
  eventSet_ = false;
  return IsEvent;
}

void
ConfigurableInputSource::setRunAndEventInfo() {
   // NOTE: numberEventsInRun < 0 means go forever in this run
   if (numberEventsInRun_ < 1 || numberEventsInThisRun_ < numberEventsInRun_) {
      // same run
      eventID_ = eventID_.next();
      if (!(numberEventsInSubRun_ < 1 || numberEventsInThisSubRun_ < numberEventsInSubRun_)) {
         // new subrun
         eventID_ = eventID_.nextSubRun();
      }
   } else {
      // new run
      eventID_ = eventID_.nextRun();
   }
   presentTime_ += timeBetweenEvents_;
   if (eventCreationDelay_ > 0) {usleep(eventCreationDelay_);}
}

// ======================================================================
