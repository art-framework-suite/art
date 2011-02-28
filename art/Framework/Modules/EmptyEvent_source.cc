namespace art {
  class EmptyEvent;
}

#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/Core/Run.h"
#include "art/Framework/Core/RunPrincipal.h"
#include "art/Framework/Core/SubRun.h"
#include "art/Framework/Core/SubRunPrincipal.h"
#include "art/Persistency/Provenance/EventAuxiliary.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/RunAuxiliary.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "art/Persistency/Provenance/Timestamp.h"
#include "boost/shared_ptr.hpp"
#include "fhiclcpp/ParameterSet.h"

#include "cpp0x/cstdint"

class art::EmptyEvent : public art::InputSource {
public:
   explicit EmptyEvent(fhicl::ParameterSet const& pset,
                       art::InputSourceDescription const& desc);
   virtual ~EmptyEvent();

   unsigned int numberEventsInRun() const { return numberEventsInRun_; }
   unsigned int numberEventsInSubRun() const { return numberEventsInSubRun_; }
   TimeValue_t presentTime() const { return presentTime_; }
   unsigned int timeBetweenEvents() const { return timeBetweenEvents_; }
   unsigned int eventCreationDelay() const { return eventCreationDelay_; }
   unsigned int numberEventsInThisRun() const { return numberEventsInThisRun_; }
   unsigned int numberEventsInThisSubRun() const { return numberEventsInThisSubRun_; }

private:
   virtual art::input::ItemType getNextItemType();
   virtual void setRunAndEventInfo();
   virtual std::auto_ptr<EventPrincipal> readEvent_();
   virtual boost::shared_ptr<SubRunPrincipal> readSubRun_();
   virtual boost::shared_ptr<RunPrincipal> readRun_();
   virtual void skip(int offset);
   virtual void rewind_();

   void setTime(TimeValue_t t) {presentTime_ = t;}
   void reallyReadEvent();

   unsigned int numberEventsInRun_;
   unsigned int numberEventsInSubRun_;
   TimeValue_t presentTime_;
   TimeValue_t origTime_;
   unsigned int timeBetweenEvents_;
   unsigned int eventCreationDelay_;  /* microseconds */

   unsigned int numberEventsInThisRun_;
   unsigned int numberEventsInThisSubRun_;
   EventID eventID_;
   EventID origEventID_;
   bool newRun_;
   bool newSubRun_;
   bool subRunSet_;
   bool eventSet_;
   std::auto_ptr<EventPrincipal> ep_;
   EventAuxiliary::ExperimentType eType_;
};  // EmptyEvent

using namespace art;

//used for defaults

namespace {
   uint32_t defaultTimeBetweenEvents() {
      static uint32_t const kNanoSecPerSec = 1000000000U;
      static uint32_t const kAveEventPerSec = 200U;
      return kNanoSecPerSec / kAveEventPerSec;
   }
}

art::EmptyEvent::EmptyEvent
(fhicl::ParameterSet const& pset, InputSourceDescription const& desc) :
   InputSource              ( pset, desc ),
   numberEventsInRun_       ( pset.get<uint32_t>("numberEventsInRun", remainingEvents()) ),
   numberEventsInSubRun_    ( pset.get<uint32_t>("numberEventsInSubRun", remainingEvents()) ),
   presentTime_             ( pset.get<uint32_t>("firstTime", 0u) ),  //time in ns
   origTime_                ( presentTime_ ),
   timeBetweenEvents_       ( pset.get<uint32_t>("timeBetweenEvents", defaultTimeBetweenEvents()) ),
   eventCreationDelay_      ( pset.get<uint32_t>("eventCreationDelay", 0u) ),
   numberEventsInThisRun_   ( 0 ),
   numberEventsInThisSubRun_( 0 ),
   eventID_                 ( ),
   origEventID_             ( ), // In body.
   newRun_                  ( true ),
   newSubRun_               ( true ),
   subRunSet_               ( false ),
   eventSet_                ( false ),
   ep_                      ( 0 ),
   eType_                   ( EventAuxiliary::Any)
{
   setTimestamp(Timestamp(presentTime_));
   // We need to map this string to the EventAuxiliary::ExperimentType enumeration
   // std::string eType = pset.get<std::string>("experimentType", std::string("Any"))),

   RunNumber_t firstRun;
   bool haveFirstRun = pset.get_if_present("firstRun", firstRun);
   SubRunNumber_t firstSubRun;
   bool haveFirstSubRun = pset.get_if_present("firstSubRun", firstSubRun);
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

art::EmptyEvent::~EmptyEvent() { }

boost::shared_ptr<RunPrincipal>
art::EmptyEvent::readRun_() {
  Timestamp ts = Timestamp(presentTime_);
  RunAuxiliary runAux(eventID_.runID(), ts, Timestamp::invalidTimestamp());
  boost::shared_ptr<RunPrincipal> runPrincipal(
      new RunPrincipal(runAux, productRegistry(), processConfiguration()));
  RunPrincipal & rp =
     const_cast<RunPrincipal &>(*runPrincipal);
  newRun_ = false;
  return runPrincipal;
}

boost::shared_ptr<SubRunPrincipal>
EmptyEvent::readSubRun_() {
   if (processingMode() == Runs) return boost::shared_ptr<SubRunPrincipal>();
   Timestamp ts = Timestamp(presentTime_);
   SubRunAuxiliary subRunAux(eventID_.subRunID(),
                             ts,
                             Timestamp::invalidTimestamp());
   boost::shared_ptr<SubRunPrincipal>
      subRunPrincipal(new SubRunPrincipal(subRunAux,
                                          productRegistry(),
                                          processConfiguration()));
   SubRun sr(*subRunPrincipal, moduleDescription());
   newSubRun_ = false;
   return subRunPrincipal;
}

std::auto_ptr<EventPrincipal>
  EmptyEvent::readEvent_() {
   assert(ep_.get() != 0 || processingMode() != RunsSubRunsAndEvents);
   return ep_;
}

void art::EmptyEvent::reallyReadEvent() {
  if (processingMode() != RunsSubRunsAndEvents) return;
  EventSourceSentry sentry(*this);
  EventAuxiliary eventAux(eventID_,
                          processGUID(),
                          Timestamp(presentTime_),
                          eType_);
  std::auto_ptr<EventPrincipal> result(
      new EventPrincipal(eventAux, productRegistry(), processConfiguration()));
  ep_ = result;
}

void art::EmptyEvent::skip(int offset)
{
  for (; offset < 0; ++offset) {
     eventID_ = eventID_.previous();
  }
  for (; offset > 0; --offset) {
     eventID_ = eventID_.next();
  }
}

void art::EmptyEvent::rewind_() {
  presentTime_ = origTime_;
  eventID_ = origEventID_;
  numberEventsInThisRun_ = 0;
  numberEventsInThisSubRun_ = 0;
  newRun_ = newSubRun_ = true;
  resetSubRunPrincipal();
  resetRunPrincipal();
}

art::input::ItemType
art::EmptyEvent::getNextItemType() {
   if (newRun_) {
      if (!eventID_.runID().isValid() ) {
         ep_.reset();
         return input::IsStop;
      }
      return input::IsRun;
   }
   if (newSubRun_) {
      return input::IsSubRun;
   }
   if(ep_.get() != 0) return input::IsEvent;
   EventID oldEventID = eventID_;
   if (!eventSet_) {
      subRunSet_ = false;
      setRunAndEventInfo();
      eventSet_ = true;
   }
   if (!eventID_.runID().isValid()) {
      ep_.reset();
      return input::IsStop;
   }
   if (oldEventID.runID() != eventID_.runID()) {
      //  New Run
      // reset these since this event is in the new run
      numberEventsInThisRun_ = 0;
      numberEventsInThisSubRun_ = 0;
      newRun_ = newSubRun_ = true;
      resetSubRunPrincipal();
      resetRunPrincipal();
      return input::IsRun;
   }
   // Same Run
   if (oldEventID.subRunID() != eventID_.subRunID()) {
      // New Subrun
      numberEventsInThisSubRun_ = 0;
      newSubRun_ = true;
      resetSubRunPrincipal();
      if (processingMode() != Runs) {
         return input::IsSubRun;
      }
   }
   ++numberEventsInThisRun_;
   ++numberEventsInThisSubRun_;
   reallyReadEvent();
   if (ep_.get() == 0) {
      return input::IsStop;
   }
   eventSet_ = false;
   return input::IsEvent;
}

void
art::EmptyEvent::setRunAndEventInfo() {
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

DEFINE_ART_INPUT_SOURCE(EmptyEvent);

