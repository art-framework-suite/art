#include "art/Framework/Core/DecrepitRelicInputSourceImplementation.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Core/EmptyEventTimestampPlugin.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/OpenRangeSetHandler.h"
#include "fhiclcpp/types/ConfigurationTable.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Persistency/Provenance/Timestamp.h"
#include "cetlib/BasicPluginFactory.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/OptionalAtom.h"
#include "fhiclcpp/types/TableFragment.h"
#include "fhiclcpp/ParameterSet.h"

#include <cstdint>
#include <memory>

namespace art {
  class EmptyEvent;
}

using DRISI = art::DecrepitRelicInputSourceImplementation;
using std::uint32_t;

class art::EmptyEvent final : public DRISI {
public:

  struct Config {

    using Name = fhicl::Name;

    fhicl::Atom<std::string> module_type { Name("module_type") };
    fhicl::TableFragment<DRISI::Config> drisi_config;
    fhicl::Atom<int> numberEventsInRun    { Name("numberEventsInRun"), drisi_config().maxEvents() };
    fhicl::Atom<int> numberEventsInSubRun { Name("numberEventsInSubRun"), drisi_config().maxSubRuns() };
    fhicl::Atom<uint32_t> eventCreationDelay   { Name("eventCreationDelay"), 0u };
    fhicl::Atom<bool> resetEventOnSubRun       { Name("resetEventOnSubRun"), true };
    fhicl::OptionalAtom<RunNumber_t>    firstRun    { Name("firstRun") };
    fhicl::OptionalAtom<SubRunNumber_t> firstSubRun { Name("firstSubRun") };
    fhicl::OptionalAtom<EventNumber_t>  firstEvent  { Name("firstEvent") };

    struct KeysToIgnore {
      std::set<std::string> operator()() const
      {
        return {"timestampPlugin", "module_label"};
      }
    };

  };

  using Parameters = fhicl::WrappedTable<Config, Config::KeysToIgnore>;

  explicit EmptyEvent(Parameters const& config,
                      InputSourceDescription & desc);

  unsigned int numberEventsInRun() const { return numberEventsInRun_; }
  unsigned int numberEventsInSubRun() const { return numberEventsInSubRun_; }
  unsigned int eventCreationDelay() const { return eventCreationDelay_; }
  unsigned int numberEventsInThisRun() const { return numberEventsInThisRun_; }
  unsigned int numberEventsInThisSubRun() const { return numberEventsInThisSubRun_; }

private:
  art::input::ItemType getNextItemType() override;
  void setRunAndEventInfo();
  std::unique_ptr<EventPrincipal> readEvent_() override;
  std::unique_ptr<SubRunPrincipal> readSubRun_() override;
  std::unique_ptr<RunPrincipal> readRun_() override;

  std::unique_ptr<RangeSetHandler> runRangeSetHandler() override;
  std::unique_ptr<RangeSetHandler> subRunRangeSetHandler() override;

  void skip(int offset) override;
  void rewind_() override;

  void beginJob() override;
  void endJob() override;

  void reallyReadEvent(bool const lastEventInSubRun);

  std::unique_ptr<EmptyEventTimestampPlugin>
  makePlugin_(fhicl::ParameterSet const & pset);

  unsigned int numberEventsInRun_;
  unsigned int numberEventsInSubRun_;
  unsigned int eventCreationDelay_;  /* microseconds */

  unsigned int numberEventsInThisRun_ {};
  unsigned int numberEventsInThisSubRun_ {};
  EventID eventID_ {};
  EventID origEventID_ {};
  bool newRun_ {true};
  bool newSubRun_ {true};
  bool subRunSet_ {false};
  bool eventSet_ {false};
  bool skipEventIncrement_ {true};
  bool resetEventOnSubRun_;
  std::unique_ptr<EventPrincipal> ep_ {};
  EventAuxiliary::ExperimentType eType_ {EventAuxiliary::Any};

  cet::BasicPluginFactory pluginFactory_ {};
  std::unique_ptr<EmptyEventTimestampPlugin> plugin_;
};  // EmptyEvent

using namespace art;

//used for defaults

art::EmptyEvent::EmptyEvent(art::EmptyEvent::Parameters const& config, InputSourceDescription & desc)
  :
  DecrepitRelicInputSourceImplementation{config().drisi_config, desc},
  numberEventsInRun_       {static_cast<uint32_t>(config().numberEventsInRun())},
  numberEventsInSubRun_    {static_cast<uint32_t>(config().numberEventsInSubRun())},
  eventCreationDelay_      {config().eventCreationDelay()},
  resetEventOnSubRun_      {config().resetEventOnSubRun()},
  plugin_                  {makePlugin_(config.get_PSet().get<fhicl::ParameterSet>("timestampPlugin", { }))}
{

  RunNumber_t firstRun{};
  bool haveFirstRun = config().firstRun(firstRun);
  SubRunNumber_t firstSubRun{};
  bool haveFirstSubRun = config().firstSubRun(firstSubRun);
  EventNumber_t firstEvent{};
  bool haveFirstEvent = config().firstEvent(firstEvent);
  RunID firstRunID = haveFirstRun?RunID(firstRun):RunID::firstRun();
  SubRunID firstSubRunID = haveFirstSubRun?SubRunID(firstRunID.run(), firstSubRun):
    SubRunID::firstSubRun(firstRunID);
  origEventID_ = haveFirstEvent?EventID(firstSubRunID.run(),
                                        firstSubRunID.subRun(),
                                        firstEvent):
    EventID::firstEvent(firstSubRunID);
  eventID_ = origEventID_;
}

std::unique_ptr<RunPrincipal>
art::EmptyEvent::readRun_()
{
  auto ts = plugin_ ?
    plugin_->doBeginRunTimestamp(eventID_.runID()) :
    Timestamp::invalidTimestamp();
  RunAuxiliary const runAux {eventID_.runID(), ts, Timestamp::invalidTimestamp()};
  newRun_ = false;
  auto rp_ptr = std::make_unique<RunPrincipal>(runAux, processConfiguration());
  if (plugin_) {
    Run const r {*rp_ptr, moduleDescription(), Consumer::non_module_context()};
    plugin_->doBeginRun(r);
  }
  return rp_ptr;
}

std::unique_ptr<RangeSetHandler>
art::EmptyEvent::runRangeSetHandler()
{
  return std::make_unique<OpenRangeSetHandler>(eventID_.run());
}

std::unique_ptr<SubRunPrincipal>
EmptyEvent::readSubRun_()
{
  if (processingMode() == Runs) return std::unique_ptr<SubRunPrincipal>{nullptr};
  auto ts = plugin_ ?
    plugin_->doBeginSubRunTimestamp(eventID_.subRunID()) :
    Timestamp::invalidTimestamp();
  SubRunAuxiliary const subRunAux {eventID_.subRunID(), ts, Timestamp::invalidTimestamp()};
  auto srp_ptr = std::make_unique<SubRunPrincipal>(subRunAux, processConfiguration());
  if (plugin_) {
    SubRun const sr {*srp_ptr, moduleDescription(), Consumer::non_module_context()};
    plugin_->doBeginSubRun(sr);
  }
  newSubRun_ = false;
  return srp_ptr;
}

std::unique_ptr<RangeSetHandler>
art::EmptyEvent::subRunRangeSetHandler()
{
  return std::make_unique<OpenRangeSetHandler>(eventID_.run());
}

std::unique_ptr<EventPrincipal>
EmptyEvent::readEvent_() {
  assert(ep_.get() != nullptr || processingMode() != RunsSubRunsAndEvents);
  return std::move(ep_);
}

void
art::EmptyEvent::
beginJob()
{
  if (plugin_) {
    plugin_->doBeginJob();
  }
}

void
art::EmptyEvent::
endJob()
{
  if (plugin_) {
    plugin_->doEndJob();
  }
}

void art::EmptyEvent::reallyReadEvent(bool const lastEventInSubRun) {
  if (processingMode() != RunsSubRunsAndEvents) return;
  auto timestamp = plugin_ ?
    plugin_->doEventTimestamp(eventID_) :
    Timestamp::invalidTimestamp();
  EventAuxiliary const eventAux{eventID_, timestamp, eType_};
  ep_ = std::make_unique<EventPrincipal>(eventAux,
                                         processConfiguration(),
                                         std::make_shared<History>(),
                                         std::make_unique<BranchMapper>(),
                                         std::make_unique<NoDelayedReader>(),
                                         lastEventInSubRun);
}

std::unique_ptr<art::EmptyEventTimestampPlugin>
art::EmptyEvent::
makePlugin_(fhicl::ParameterSet const & pset)
{
  std::unique_ptr<art::EmptyEventTimestampPlugin> result;
  try {
    if (!pset.is_empty()) {
      auto const libspec = pset.get<std::string>("plugin_type");
      auto const pluginType = pluginFactory_.pluginType(libspec);
      if (pluginType == cet::PluginTypeDeducer<EmptyEventTimestampPlugin>::value) {
        result = pluginFactory_.makePlugin<std::unique_ptr<EmptyEventTimestampPlugin> >(libspec, pset);
      } else {
        throw Exception(errors::Configuration, "EmptyEvent: ")
          << "unrecognized plugin type "
          << pluginType
          << "for plugin "
          << libspec
          << ".\n";
      }
    }
  } catch (cet::exception & e) {
    throw Exception(errors::Configuration, "EmptyEvent: ", e)
      << "Exception caught while processing plugin spec.\n";
  }
  return result;
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
  if (plugin_) {
    plugin_->doRewind();
  }
  setTimestamp(Timestamp::invalidTimestamp());
  eventID_ = origEventID_;
  skipEventIncrement_ = true;
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
  if(ep_.get() != nullptr) return input::IsEvent;
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
  bool const lastEventInSubRun = numberEventsInThisSubRun_ == numberEventsInSubRun_;
  reallyReadEvent(lastEventInSubRun);
  if (ep_.get() == nullptr) {
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
    if (!(numberEventsInSubRun_ < 1 || numberEventsInThisSubRun_ < numberEventsInSubRun_)) {
      // new subrun
      if (resetEventOnSubRun_) {
        eventID_ = eventID_.nextSubRun(origEventID_.event());
      } else {
        eventID_ = eventID_.nextSubRun(eventID_.next().event());
      }
    } else if (skipEventIncrement_) { // For first event, rewind etc.
      skipEventIncrement_ = false;
    } else {
      eventID_ = eventID_.next();
    }
  } else {
    // new run
    eventID_ = EventID(eventID_.nextRun().run(), origEventID_.subRun(), origEventID_.event());
  }
  if (eventCreationDelay_ > 0) {usleep(eventCreationDelay_);}
}

DEFINE_ART_INPUT_SOURCE(EmptyEvent)
