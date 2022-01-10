// vim: set sw=2 expandtab :

#include "art/Framework/Core/DecrepitRelicInputSourceImplementation.h"
#include "art/Framework/Core/EmptyEventTimestampPlugin.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/Core/fwd.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/OpenRangeSetHandler.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Provenance/ModuleContext.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Persistency/Provenance/Timestamp.h"
#include "cetlib/BasicPluginFactory.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/ConfigurationTable.h"
#include "fhiclcpp/types/OptionalAtom.h"
#include "fhiclcpp/types/OptionalDelegatedParameter.h"
#include "fhiclcpp/types/TableFragment.h"

#include <cassert>
#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>

using namespace fhicl;
using namespace std;
using namespace std::chrono_literals;
using std::chrono::steady_clock;
using DRISI = art::DecrepitRelicInputSourceImplementation;

namespace art {

  class EmptyEvent final : public DRISI {
  public:
    struct Config {

      Atom<std::string> module_type{Name("module_type")};
      TableFragment<DRISI::Config> drisi_config;
      Atom<int> numberEventsInRun{Name("numberEventsInRun"),
                                  drisi_config().maxEvents()};
      Atom<int> numberEventsInSubRun{Name("numberEventsInSubRun"),
                                     drisi_config().maxSubRuns()};
      Atom<uint32_t> maxTime{
        Name("maxTime"),
        Comment(
          "If specified, the 'maxTime' parameter indicates the maximum "
          "allowed\n"
          "wall-clock time (in seconds) for which new events may be created.\n"
          "This option is mutually exclusive with the 'maxEvents' and "
          "'maxSubRuns'\n"
          "configuration parameters."),
        std::numeric_limits<uint32_t>::max()};
      Atom<uint32_t> eventCreationDelay{
        Name("eventCreationDelay"),
        Comment("The 'eventCreationDelay' parameter is an integral value\n"
                "in the range [0, 1000000), corresponding to microseconds.\n"
                "If specified, the input source will sleep for the specified "
                "duration\n"
                "of time before each new event, subrun, or run is created.\n"),
        0u};
      Atom<bool> resetEventOnSubRun{Name("resetEventOnSubRun"), true};
      OptionalAtom<RunNumber_t> firstRun{Name("firstRun")};
      OptionalAtom<SubRunNumber_t> firstSubRun{Name("firstSubRun")};
      OptionalAtom<EventNumber_t> firstEvent{Name("firstEvent")};
      OptionalDelegatedParameter timestampPlugin{
        Name("timestampPlugin"),
        Comment(
          "The 'timestampPlugin' parameter must be a FHiCL table\n"
          "of the form:\n\n"
          "  timestampPlugin: {\n"
          "    plugin_type: <plugin specification>\n"
          "    ...\n"
          "  }\n\n"
          "See the notes in art/Framework/Core/EmptyEventTimestampPlugin.h\n"
          "for more details.")};

      struct KeysToIgnore {
        std::set<std::string>
        operator()() const
        {
          return {"module_label"};
        }
      };
    };

    using Parameters = fhicl::WrappedTable<Config, Config::KeysToIgnore>;

    EmptyEvent(Parameters const& config, InputSourceDescription& desc);

    EmptyEvent(EmptyEvent const&) = delete;
    EmptyEvent(EmptyEvent&&) = delete;
    EmptyEvent& operator=(EmptyEvent const&) = delete;
    EmptyEvent& operator=(EmptyEvent&&) = delete;

  private:
    unique_ptr<RangeSetHandler> runRangeSetHandler() override;
    unique_ptr<RangeSetHandler> subRunRangeSetHandler() override;
    input::ItemType getNextItemType() override;
    unique_ptr<RunPrincipal> readRun_() override;
    unique_ptr<SubRunPrincipal> readSubRun_(
      cet::exempt_ptr<RunPrincipal const>) override;
    unique_ptr<EventPrincipal> readEvent_() override;
    unique_ptr<EmptyEventTimestampPlugin> makePlugin_(
      OptionalDelegatedParameter const& maybeConfig);
    void beginJob() override;
    void endJob() override;
    void skip(int offset) override;
    void rewind_() override;

    unsigned const numberEventsInRun_;
    unsigned const numberEventsInSubRun_;
    steady_clock::time_point const beginTime_{steady_clock::now()};
    std::chrono::seconds const maxTime_;
    std::chrono::microseconds const eventCreationDelay_;
    unsigned numberEventsInThisRun_{};
    unsigned numberEventsInThisSubRun_{};
    EventID origEventID_{};
    EventID eventID_{};
    bool firstTime_{true};
    bool newFile_{true};
    bool newRun_{true};
    bool newSubRun_{true};
    bool const resetEventOnSubRun_;
    cet::BasicPluginFactory pluginFactory_{};
    unique_ptr<EmptyEventTimestampPlugin> plugin_;
  };

} // namespace art

art::EmptyEvent::EmptyEvent(Parameters const& config,
                            InputSourceDescription& desc)
  : DecrepitRelicInputSourceImplementation{config().drisi_config,
                                           desc.moduleDescription}
  , numberEventsInRun_{static_cast<uint32_t>(config().numberEventsInRun())}
  , numberEventsInSubRun_{static_cast<uint32_t>(
      config().numberEventsInSubRun())}
  , maxTime_{config().maxTime()}
  , eventCreationDelay_{config().eventCreationDelay()}
  , resetEventOnSubRun_{config().resetEventOnSubRun()}
  , plugin_{makePlugin_(config().timestampPlugin)}
{
  // Additional configuration checking which is cumbersome to do with
  // the FHiCL validation system.
  auto const& pset = config.get_PSet();
  if (pset.has_key("maxTime") &&
      (pset.has_key("maxEvents") || pset.has_key("maxSubRuns"))) {
    throw Exception{
      errors::Configuration,
      "An error occurred while configuring the EmptyEvent source.\n"}
      << "The 'maxTime' parameter cannot be used with the 'maxEvents' or "
         "'maxSubRuns' parameters.\n"
         "Type 'art --print-description EmptyEvent' for the allowed "
         "configuration.\n";
  }

  RunNumber_t firstRun{};
  bool haveFirstRun = config().firstRun(firstRun);
  SubRunNumber_t firstSubRun{};
  bool haveFirstSubRun = config().firstSubRun(firstSubRun);
  EventNumber_t firstEvent{};
  bool haveFirstEvent = config().firstEvent(firstEvent);
  RunID firstRunID = haveFirstRun ? RunID(firstRun) : RunID::firstRun();
  SubRunID firstSubRunID = haveFirstSubRun ?
                             SubRunID(firstRunID.run(), firstSubRun) :
                             SubRunID::firstSubRun(firstRunID);
  origEventID_ =
    haveFirstEvent ?
      EventID(firstSubRunID.run(), firstSubRunID.subRun(), firstEvent) :
      EventID::firstEvent(firstSubRunID);
  eventID_ = origEventID_;
}

art::input::ItemType
art::EmptyEvent::getNextItemType()
{
  // Trigger framework stop if max allowed time is exceeded.
  // N.B. Since the begin time corresponds to source construction and
  // not the actual event loop, there will be minor differences wrt
  // the time reported for executing a given job.
  if (steady_clock::now() - beginTime_ > maxTime_) {
    return input::IsStop;
  }
  // First check for sanity because skip(offset) can be abused and so can the
  // ctor.
  if (!eventID_.runID().isValid()) {
    return input::IsStop;
  }
  if (!eventID_.subRunID().isValid()) {
    return input::IsStop;
  }
  if (!eventID_.isValid()) {
    return input::IsStop;
  }
  if (newFile_) {
    newFile_ = false;
    return input::IsFile;
  }
  if (newRun_) {
    newRun_ = false;
    if (eventCreationDelay_ > 0ms) {
      std::this_thread::sleep_for(eventCreationDelay_);
    }
    return input::IsRun;
  }
  if (newSubRun_) {
    newSubRun_ = false;
    if (eventCreationDelay_ > 0ms) {
      std::this_thread::sleep_for(eventCreationDelay_);
    }
    return input::IsSubRun;
  }
  if ((numberEventsInRun_ > 0) &&
      (numberEventsInRun_ <= numberEventsInThisRun_)) {
    // Time to switch runs.
    newRun_ = false;
    newSubRun_ = true;
    numberEventsInThisRun_ = 0;
    numberEventsInThisSubRun_ = 0;
    eventID_ = EventID(
      eventID_.nextRun().run(), origEventID_.subRun(), origEventID_.event());
    firstTime_ = true;
    if (eventCreationDelay_ > 0ms) {
      std::this_thread::sleep_for(eventCreationDelay_);
    }
    return input::IsRun;
  }
  if ((numberEventsInSubRun_ > 0) &&
      (numberEventsInSubRun_ <= numberEventsInThisSubRun_)) {
    // Time to switch subruns.
    newRun_ = false;
    newSubRun_ = false;
    numberEventsInThisSubRun_ = 0;
    if (resetEventOnSubRun_) {
      eventID_ = eventID_.nextSubRun(origEventID_.event());
    } else {
      eventID_ = eventID_.nextSubRun(eventID_.next().event());
    }
    firstTime_ = true;
    if (eventCreationDelay_ > 0ms) {
      std::this_thread::sleep_for(eventCreationDelay_);
    }
    return input::IsSubRun;
  }
  // same run and subrun
  if (!firstTime_) {
    eventID_ = eventID_.next();
    if (!eventID_.runID().isValid()) {
      return input::IsStop;
    }
  }
  firstTime_ = false;
  ++numberEventsInThisRun_;
  ++numberEventsInThisSubRun_;
  if (eventCreationDelay_ > 0ms) {
    std::this_thread::sleep_for(eventCreationDelay_);
  }
  return input::IsEvent;
}

unique_ptr<art::RunPrincipal>
art::EmptyEvent::readRun_()
{
  unique_ptr<RunPrincipal> result;
  auto ts = plugin_ ? plugin_->doBeginRunTimestamp(eventID_.runID()) :
                      Timestamp::invalidTimestamp();
  RunAuxiliary const runAux{
    eventID_.runID(), ts, Timestamp::invalidTimestamp()};
  result = make_unique<RunPrincipal>(runAux, processConfiguration(), nullptr);
  assert(result.get() != nullptr);
  if (plugin_) {
    ModuleContext const mc{moduleDescription()};
    auto const r = Run::make(std::as_const(*result), mc);
    plugin_->doBeginRun(r);
  }
  return result;
}

unique_ptr<art::SubRunPrincipal>
art::EmptyEvent::readSubRun_(cet::exempt_ptr<RunPrincipal const> rp)
{
  unique_ptr<SubRunPrincipal> result;
  if (processingMode() == Runs) {
    return result;
  }
  auto ts = plugin_ ? plugin_->doBeginSubRunTimestamp(eventID_.subRunID()) :
                      Timestamp::invalidTimestamp();
  SubRunAuxiliary const subRunAux{
    eventID_.subRunID(), ts, Timestamp::invalidTimestamp()};
  result =
    make_unique<SubRunPrincipal>(subRunAux, processConfiguration(), nullptr);
  assert(result.get() != nullptr);
  result->setRunPrincipal(rp);
  if (plugin_) {
    ModuleContext const mc{moduleDescription()};
    auto const sr = SubRun::make(std::as_const(*result), mc);
    plugin_->doBeginSubRun(sr);
  }
  return result;
}

unique_ptr<art::EventPrincipal>
art::EmptyEvent::readEvent_()
{
  unique_ptr<EventPrincipal> result;
  if (processingMode() != RunsSubRunsAndEvents) {
    return result;
  }
  auto timestamp = plugin_ ? plugin_->doEventTimestamp(eventID_) :
                             Timestamp::invalidTimestamp();
  EventAuxiliary const eventAux{
    eventID_, timestamp, false, EventAuxiliary::Any};
  result = make_unique<EventPrincipal>(eventAux,
                                       processConfiguration(),
                                       nullptr,
                                       make_unique<History>(),
                                       make_unique<NoDelayedReader>(),
                                       numberEventsInThisSubRun_ ==
                                         numberEventsInSubRun_);
  assert(result.get() != nullptr);
  return result;
}

unique_ptr<art::RangeSetHandler>
art::EmptyEvent::runRangeSetHandler()
{
  return make_unique<OpenRangeSetHandler>(eventID_.run());
}

unique_ptr<art::RangeSetHandler>
art::EmptyEvent::subRunRangeSetHandler()
{
  return make_unique<OpenRangeSetHandler>(eventID_.run());
}

void
art::EmptyEvent::beginJob()
{
  if (plugin_) {
    plugin_->doBeginJob();
  }
}

void
art::EmptyEvent::endJob()
{
  if (plugin_) {
    plugin_->doEndJob();
  }
}

std::unique_ptr<art::EmptyEventTimestampPlugin>
art::EmptyEvent::makePlugin_(OptionalDelegatedParameter const& maybeConfig)
{
  std::unique_ptr<EmptyEventTimestampPlugin> result;
  try {
    ParameterSet pset;
    if (maybeConfig.get_if_present(pset)) {
      auto const libspec = pset.get<std::string>("plugin_type");
      auto const pluginType = pluginFactory_.pluginType(libspec);
      if (pluginType ==
          cet::PluginTypeDeducer<EmptyEventTimestampPlugin>::value) {
        result = pluginFactory_.makePlugin<decltype(result)>(libspec, pset);
      } else {
        throw Exception(errors::Configuration, "EmptyEvent: ")
          << "unrecognized plugin type " << pluginType << "for plugin "
          << libspec << ".\n";
      }
    }
  }
  catch (cet::exception& e) {
    throw Exception(errors::Configuration, "EmptyEvent: ", e)
      << "Exception caught while processing plugin spec.\n";
  }
  return result;
}

void
art::EmptyEvent::skip(int offset)
{
  for (; offset < 0; ++offset) {
    eventID_ = eventID_.previous();
  }
  for (; offset > 0; --offset) {
    eventID_ = eventID_.next();
  }
}

void
art::EmptyEvent::rewind_()
{
  if (plugin_) {
    plugin_->doRewind();
  }
  firstTime_ = true;
  newFile_ = true;
  newRun_ = true;
  newSubRun_ = true;
  numberEventsInThisRun_ = 0;
  numberEventsInThisSubRun_ = 0;
  eventID_ = origEventID_;
}

DEFINE_ART_INPUT_SOURCE(art::EmptyEvent)
