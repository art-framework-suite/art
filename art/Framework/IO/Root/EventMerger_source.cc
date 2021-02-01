#include "art/Framework/Core/DecrepitRelicInputSourceImplementation.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/IO/Root/EventMergerFileSequence.h"
#include "art/Framework/IO/Root/FastCloningInfoProvider.h"
#include "art/Framework/IO/Root/Inputfwd.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RangeSetHandler.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/types/ConfigurationTable.h"
#include "fhiclcpp/types/KeysToIgnore.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/TableFragment.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "TTreeCache.h"

#include <array>
#include <cassert>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

namespace art {

  class EventMerger final : public DecrepitRelicInputSourceImplementation {
  public:
    struct Config {
      fhicl::Atom<std::string> module_type{fhicl::Name("module_type")};
      fhicl::TableFragment<DecrepitRelicInputSourceImplementation::Config>
        drisi_config;
      fhicl::TableFragment<EventMergerFileSequence::Config> fs_config;

      struct KeysToIgnore {
        std::set<std::string>
        operator()() const
        {
          return {"module_label"};
        }
      };
    };

    using Parameters = fhicl::WrappedTable<Config, Config::KeysToIgnore>;

    EventMerger(Parameters const&, InputSourceDescription&);
    using DecrepitRelicInputSourceImplementation::runPrincipal;

  private:
    class AccessState {
    public:
      enum State {
        SEQUENTIAL = 0,
        SEEKING_FILE,   // 1
        SEEKING_RUN,    // 2
        SEEKING_SUBRUN, // 3
        SEEKING_EVENT   // 4
      };

    public:
      State
      state() const
      {
        return state_;
      }
      void
      setState(State const state)
      {
        state_ = state;
      }
      void
      resetState()
      {
        state_ = SEQUENTIAL;
      }
      EventID const&
      lastReadEventID() const
      {
        return lastReadEventID_;
      }
      void
      setLastReadEventID(EventID const& eid)
      {
        lastReadEventID_ = eid;
      }

      EventID const&
      wantedEventID() const
      {
        return wantedEventID_;
      }
      void
      setWantedEventID(EventID const& eid)
      {
        wantedEventID_ = eid;
      }
      std::shared_ptr<RootInputFile>
      rootFileForLastReadEvent() const
      {
        return rootFileForLastReadEvent_;
      }
      void
      setRootFileForLastReadEvent(std::shared_ptr<RootInputFile> const& ptr)
      {
        rootFileForLastReadEvent_ = ptr;
      }

    private:
      State state_{SEQUENTIAL};
      EventID lastReadEventID_{};
      std::shared_ptr<RootInputFile> rootFileForLastReadEvent_{};
      EventID wantedEventID_{};
    }; // class AccessState

    EventMergerFileSequence fileSequence_;
    AccessState accessState_{};

    void finish() override;
    input::ItemType nextItemType() override;
    using DecrepitRelicInputSourceImplementation::readEvent;
    std::unique_ptr<EventPrincipal> readEvent(
      cet::exempt_ptr<SubRunPrincipal const>) override;
    std::unique_ptr<EventPrincipal> readEvent_() override;
    std::unique_ptr<EventPrincipal> readEvent_(
      cet::exempt_ptr<SubRunPrincipal const>);
    std::unique_ptr<SubRunPrincipal> readSubRun(
      cet::exempt_ptr<RunPrincipal const>) override;
    std::unique_ptr<SubRunPrincipal> readSubRun_() override;
    std::unique_ptr<RunPrincipal> readRun() override;
    std::unique_ptr<RunPrincipal> readRun_() override;
    std::unique_ptr<FileBlock> readFile() override;
    std::unique_ptr<FileBlock> readFile_() override;
    std::unique_ptr<RangeSetHandler> runRangeSetHandler() override;
    std::unique_ptr<RangeSetHandler> subRunRangeSetHandler() override;
    void closeFile_() override;
    void endJob() override;
    input::ItemType getNextItemType() override;
    void rewind_() override;
  }; // class EventMerger

} // namespace art

using namespace art;

EventMerger::EventMerger(EventMerger::Parameters const& config,
                         InputSourceDescription& desc)
  : DecrepitRelicInputSourceImplementation{config().drisi_config,
                                           desc.moduleDescription}
  , fileSequence_{config().fs_config,
                  FastCloningInfoProvider(cet::exempt_ptr<EventMerger>(this)),
                  processingMode(),
                  desc.productRegistry,
                  processConfiguration()}
{}

void
EventMerger::endJob()
{
  fileSequence_.endJob();
}

void
EventMerger::closeFile_()
{
  fileSequence_.closeFiles_();
}

void
EventMerger::rewind_()
{}

input::ItemType
EventMerger::getNextItemType()
{
  return fileSequence_.getNextItemType();
}

void
EventMerger::finish()
{}

input::ItemType
EventMerger::nextItemType()
{
  switch (accessState_.state()) {
    case AccessState::SEQUENTIAL:
      return DecrepitRelicInputSourceImplementation::nextItemType();
    case AccessState::SEEKING_FILE:
      return input::IsFile;
    case AccessState::SEEKING_RUN:
      setRunPrincipal(
        fileSequence_.readIt(accessState_.wantedEventID().runID()));
      return input::IsRun;
    case AccessState::SEEKING_SUBRUN:
      setSubRunPrincipal(
        fileSequence_.readIt(accessState_.wantedEventID().subRunID()));
      return input::IsSubRun;
    case AccessState::SEEKING_EVENT: {
      auto const wantedEventID = accessState_.wantedEventID();
      setEventPrincipal(fileSequence_.readIt(wantedEventID, true));
      accessState_.setLastReadEventID(wantedEventID);
      accessState_.setRootFileForLastReadEvent(
        fileSequence_.rootFileForLastReadEvent());
      return input::IsEvent;
    }
    default:
      throw Exception(errors::LogicError)
        << "EventMergerSource::nextItemType encountered an "
           "unknown AccessState.\n";
  }
}

std::unique_ptr<FileBlock>
EventMerger::readFile()
{
  switch (accessState_.state()) {
    case AccessState::SEQUENTIAL:
      return DecrepitRelicInputSourceImplementation::readFile();
    case AccessState::SEEKING_FILE:
      accessState_.setState(AccessState::SEEKING_RUN);
      return readFile_();
    default:
      throw Exception(errors::LogicError)
        << "EventMergerSource::readFile encountered an "
           "unknown or inappropriate AccessState.\n";
  }
}

std::unique_ptr<FileBlock>
EventMerger::readFile_()
{
  return fileSequence_.readFile_();
}

std::unique_ptr<RunPrincipal>
EventMerger::readRun()
{
  switch (accessState_.state()) {
    case AccessState::SEQUENTIAL:
      return DecrepitRelicInputSourceImplementation::readRun();
    case AccessState::SEEKING_RUN:
      accessState_.setState(AccessState::SEEKING_SUBRUN);
      return runPrincipal();
    default:
      throw Exception(errors::LogicError)
        << "EventMergerSource::readRun encountered an "
           "unknown or inappropriate AccessState.\n";
  }
}

std::unique_ptr<RunPrincipal>
EventMerger::readRun_()
{
  return fileSequence_.readRun_();
}

std::unique_ptr<RangeSetHandler>
EventMerger::runRangeSetHandler()
{
  return fileSequence_.runRangeSetHandler();
}

std::unique_ptr<SubRunPrincipal>
EventMerger::readSubRun(cet::exempt_ptr<RunPrincipal const> rp)
{
  switch (accessState_.state()) {
    case AccessState::SEQUENTIAL:
      return DecrepitRelicInputSourceImplementation::readSubRun(rp);
    case AccessState::SEEKING_SUBRUN:
      accessState_.setState(AccessState::SEEKING_EVENT);
      return subRunPrincipal();
    default:
      throw Exception(errors::LogicError)
        << "EventMergerSource::readSubRun encountered an "
           "unknown or inappropriate AccessState.\n";
  }
}

std::unique_ptr<SubRunPrincipal>
EventMerger::readSubRun_()
{
  return fileSequence_.readSubRun_();
}

std::unique_ptr<RangeSetHandler>
EventMerger::subRunRangeSetHandler()
{
  return fileSequence_.subRunRangeSetHandler();
}

std::unique_ptr<EventPrincipal>
EventMerger::readEvent(cet::exempt_ptr<SubRunPrincipal const> srp)
{
  return readEvent_(srp);
}

std::unique_ptr<EventPrincipal>
EventMerger::readEvent_(cet::exempt_ptr<SubRunPrincipal const> srp)
{
  switch (accessState_.state()) {
    case AccessState::SEQUENTIAL:
      return DecrepitRelicInputSourceImplementation::readEvent(srp);
    case AccessState::SEEKING_EVENT:
      accessState_.resetState();
      return eventPrincipal();
    default:
      throw Exception(errors::LogicError)
        << "EventMergerSource::readEvent encountered an "
           "unknown or inappropriate AccessState.\n";
  }
}

std::unique_ptr<EventPrincipal>
EventMerger::readEvent_()
{
  std::unique_ptr<EventPrincipal> result;
  if (!result.get()) {
    result = fileSequence_.readEvent_();
  }
  if (result.get()) {
    accessState_.setLastReadEventID(result->id());
    accessState_.setRootFileForLastReadEvent(
      fileSequence_.rootFileForLastReadEvent());
  }
  return result;
}

DEFINE_ART_INPUT_SOURCE(EventMerger)
