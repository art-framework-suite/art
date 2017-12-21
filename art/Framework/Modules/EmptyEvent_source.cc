// vim: set sw=2 expandtab :

#include "art/Framework/Core/DecrepitRelicInputSourceImplementation.h"
#include "art/Framework/Core/EmptyEventTimestampPlugin.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/OpenRangeSetHandler.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
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
#include "fhiclcpp/types/TableFragment.h"

#include <cstdint>
#include <memory>

using namespace std;

namespace art {

  class EmptyEvent final : public DecrepitRelicInputSourceImplementation {

  public: // CONFIGURATION
    struct Config {

      using Name = fhicl::Name;

      fhicl::Atom<string> module_type{Name("module_type")};
      fhicl::TableFragment<DecrepitRelicInputSourceImplementation::Config>
        drisi_config;
      fhicl::Atom<int> numberEventsInRun{Name("numberEventsInRun"),
                                         drisi_config().maxEvents()};
      fhicl::Atom<int> numberEventsInSubRun{Name("numberEventsInSubRun"),
                                            drisi_config().maxSubRuns()};
      fhicl::Atom<uint32_t> eventCreationDelay{Name("eventCreationDelay"), 0u};
      fhicl::Atom<bool> resetEventOnSubRun{Name("resetEventOnSubRun"), true};
      fhicl::OptionalAtom<RunNumber_t> firstRun{Name("firstRun")};
      fhicl::OptionalAtom<SubRunNumber_t> firstSubRun{Name("firstSubRun")};
      fhicl::OptionalAtom<EventNumber_t> firstEvent{Name("firstEvent")};

      struct KeysToIgnore {
        set<string>
        operator()() const
        {
          return {"timestampPlugin", "module_label"};
        }
      };
    };

    using Parameters = fhicl::WrappedTable<Config, Config::KeysToIgnore>;

  public: // MEMBER FUNCTIONS -- Special Member Functions
    EmptyEvent(Parameters const& config, InputSourceDescription& desc);

    EmptyEvent(EmptyEvent const&) = delete;
    EmptyEvent(EmptyEvent&&) = delete;

    EmptyEvent& operator=(EmptyEvent const&) = delete;
    EmptyEvent& operator=(EmptyEvent&&) = delete;

  public: // MEMBER FUNCTIONS
    unsigned
    numberEventsInRun() const
    {
      return numberEventsInRun_;
    }

    unsigned
    numberEventsInSubRun() const
    {
      return numberEventsInSubRun_;
    }

    unsigned
    eventCreationDelay() const
    {
      return eventCreationDelay_;
    }

    unsigned
    numberEventsInThisRun() const
    {
      return numberEventsInThisRun_;
    }

    unsigned
    numberEventsInThisSubRun() const
    {
      return numberEventsInThisSubRun_;
    }

  public: // MEMBER FUNCTIONS -- Serial Access Interface -- NOT IMPLEMENTED
          // Not Implemented
          // virtual
          // input::ItemType
          // nextItemType() override;
    // Not Implemented
    // Open next file
    // virtual
    // std::unique_ptr<FileBlock>
    // readFile(UpdateOutputCallbacks&) override;

    // Not Implemented
    // Close current file
    // virtual
    // void
    // closeFile() override;

    // Not Implemented
    // Read next run.
    // virtual
    // std::unique_ptr<RunPrincipal>
    // readRun() override;

    // Not Implemented
    // Read next subRun
    // virtual
    // std::unique_ptr<SubRunPrincipal>
    // readSubRun(cet::exempt_ptr<RunPrincipal const>) override;

    // Not Implemented
    // Read next event
    // Indicate inability to get a new event by returning a null unique_ptr.
    // virtual
    // std::unique_ptr<EventPrincipal>
    // readEvent(cet::exempt_ptr<SubRunPrincipal const>) override;

  private: // MEMBER FUNCTIONS -- Serial Access Interface
    virtual unique_ptr<RangeSetHandler> runRangeSetHandler() override;

    virtual unique_ptr<RangeSetHandler> subRunRangeSetHandler() override;

  public: // MEMBER FUNCTIONS -- Job Interface -- NOT IMPLEMENTED
          // Not Implemented
          // Called by framework at beginning of job
          // virtual
          // void
          // doBeginJob() override;
    // Not Implemented
    // Called by framework at end of job
    // virtual
    // void
    // doEndJob() override;

  public: // MEMBER FUNCTIONS -- Random Access Interface -- NOT IMPLEMENTED
          // Not Implemented
          // Read a specific event
          // virtual
          // std::unique_ptr<EventPrincipal>
          // readEvent(EventID const&) override;
    // Not Implemented
    // Skip the number of events specified.
    // Offset may be negative.
    // virtual
    // void
    // skipEvents(int offset) override;

    // Not Implemented
    // Begin again at the first event
    // virtual
    // void
    // rewind() override;

  private
    : // MEMBER FUNCTIONS -- DecrepitRelicInputSourceImplementation required
      // interface
    input::ItemType getNextItemType() override;

    unique_ptr<RunPrincipal> readRun_() override;

    unique_ptr<SubRunPrincipal> readSubRun_(
      cet::exempt_ptr<RunPrincipal const>) override;

    unique_ptr<EventPrincipal> readEvent_() override;

  private
    : // MEMBER FUNCTIONS -- DecrepitRelicInputSourceImplementation interface
      // -- NOT IMPLEMENTED
    // Not Implemented
    // virtual
    // std::unique_ptr<FileBlock>
    // readFile_();

    // Not Implemented
    // virtual
    // void
    // closeFile_();

  private
    : // MEMBER FUNCTIONS -- DecrepitRelicInputSourceImplementation interface
    void beginJob() override;

    void endJob() override;

  private
    : // MEMBER FUNCTIONS -- DecrepitRelicInputSourceImplementation interface
    void skip(int offset) override;

    void rewind_() override;

    std::unique_ptr<EmptyEventTimestampPlugin> makePlugin_(
      fhicl::ParameterSet const& pset);

  private: // MEMBER DATA
    unsigned const numberEventsInRun_;
    unsigned const numberEventsInSubRun_;
    // microseconds
    unsigned const eventCreationDelay_;
    unsigned numberEventsInThisRun_{};
    unsigned numberEventsInThisSubRun_{};
    EventID origEventID_{};
    EventID eventID_{};
    bool firstTime_{true};
    bool newFile_{true};
    bool newRun_{true};
    bool newSubRun_{true};
    bool const resetEventOnSubRun_;
    EventAuxiliary::ExperimentType eType_{EventAuxiliary::Any};
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
  , eventCreationDelay_{config().eventCreationDelay()}
  , resetEventOnSubRun_{config().resetEventOnSubRun()}
  , plugin_{makePlugin_(
      config.get_PSet().get<fhicl::ParameterSet>("timestampPlugin", {}))}
{
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
    if (eventCreationDelay_ > 0) {
      usleep(eventCreationDelay_);
    }
    return input::IsRun;
  }
  if (newSubRun_) {
    newSubRun_ = false;
    if (eventCreationDelay_ > 0) {
      usleep(eventCreationDelay_);
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
    if (eventCreationDelay_ > 0) {
      usleep(eventCreationDelay_);
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
    if (eventCreationDelay_ > 0) {
      usleep(eventCreationDelay_);
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
  if (eventCreationDelay_ > 0) {
    usleep(eventCreationDelay_);
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
    Run const r{*result, moduleDescription(), TypeLabelLookup_t{}};
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
    SubRun const sr{*result, moduleDescription(), TypeLabelLookup_t{}};
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
  EventAuxiliary const eventAux{eventID_, timestamp, eType_};
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
art::EmptyEvent::makePlugin_(fhicl::ParameterSet const& pset)
{
  std::unique_ptr<EmptyEventTimestampPlugin> result;
  try {
    if (!pset.is_empty()) {
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
