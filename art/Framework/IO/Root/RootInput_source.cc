#include "art/Framework/IO/Root/RootInput.h"
// vim: set sw=2:

#include "TTreeCache.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/IO/Root/FastCloningInfoProvider.h"
#include "art/Framework/IO/Root/RootInputFileSequence.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RangeSetHandler.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Utilities/forceRootDynamicPathRescan.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Utilities/Exception.h"

#include <cassert>
#include <memory>
#include <set>

using namespace art;

RootInput::AccessState::AccessState()
  : state_(SEQUENTIAL)
  , lastReadEventID_()
  , rootFileForLastReadEvent_()
  , wantedEventID_()
{}

void
RootInput::AccessState::setState(State state)
{
  state_ = state;
}

void
RootInput::AccessState::setLastReadEventID(EventID const& eid)
{
  lastReadEventID_ = eid;
}

void
RootInput::AccessState::setWantedEventID(EventID const& eid)
{
  wantedEventID_ = eid;
}

void
RootInput::AccessState::setRootFileForLastReadEvent(
  std::shared_ptr<RootInputFile> const& ptr)
{
  rootFileForLastReadEvent_ = ptr;
}

RootInput::RootInput(RootInput::Parameters const& config,
                     InputSourceDescription& desc)
  : DecrepitRelicInputSourceImplementation{config().drisi_config,
                                           desc.moduleDescription}
  , catalog_{config().ifc_config}
  , primaryFileSequence_{std::make_unique<RootInputFileSequence>(
      config().rifs_config,
      catalog_,
      FastCloningInfoProvider(cet::exempt_ptr<RootInput>(this)),
      processingMode(),
      desc.productRegistry,
      processConfiguration())}
  , accessState_{}
{
  forceRootDynamicPathRescan();
}

void
RootInput::endJob()
{
  primaryFileSequence_->endJob();
}

void
RootInput::closeFile_()
{
  primaryFileSequence_->closeFile_();
}

void
RootInput::rewind_()
{
  // Rewind to before the first event that was read.
  accessState_.resetState();
  primaryFileSequence_->rewind_();
}

input::ItemType
RootInput::getNextItemType()
{
  return primaryFileSequence_->getNextItemType();
}

void
RootInput::finish()
{
  primaryFileSequence_->finish();
}

input::ItemType
RootInput::nextItemType()
{
  switch (accessState_.state()) {
    case AccessState::SEQUENTIAL:
      return DecrepitRelicInputSourceImplementation::nextItemType();
    case AccessState::SEEKING_FILE:
      return input::IsFile;
    case AccessState::SEEKING_RUN:
      setRunPrincipal(
        primaryFileSequence_->readIt(accessState_.wantedEventID().runID()));
      return input::IsRun;
    case AccessState::SEEKING_SUBRUN:
      // RunPrincipal has been handed off to the EventProcessor by this point.
      // Used the cached pointer.
      setSubRunPrincipal(primaryFileSequence_->readIt(
        accessState_.wantedEventID().subRunID(), runPrincipalExemptPtr()));
      return input::IsSubRun;
    case AccessState::SEEKING_EVENT: {
      auto const wantedEventID = accessState_.wantedEventID();
      setEventPrincipal(primaryFileSequence_->readIt(wantedEventID, true));
      accessState_.setLastReadEventID(wantedEventID);
      accessState_.setRootFileForLastReadEvent(
        primaryFileSequence_->rootFileForLastReadEvent());
      return input::IsEvent;
    }
    default:
      throw Exception(errors::LogicError)
        << "RootInputSource::nextItemType encountered an "
           "unknown AccessState.\n";
  }
}

std::unique_ptr<FileBlock>
RootInput::readFile()
{
  switch (accessState_.state()) {
    case AccessState::SEQUENTIAL:
      return DecrepitRelicInputSourceImplementation::readFile();
    case AccessState::SEEKING_FILE:
      accessState_.setState(AccessState::SEEKING_RUN);
      return readFile_();
    default:
      throw Exception(errors::LogicError)
        << "RootInputSource::readFile encountered an "
           "unknown or inappropriate AccessState.\n";
  }
}

std::unique_ptr<FileBlock>
RootInput::readFile_()
{
  return primaryFileSequence_->readFile_();
}

std::unique_ptr<RunPrincipal>
RootInput::readRun()
{
  switch (accessState_.state()) {
    case AccessState::SEQUENTIAL:
      return DecrepitRelicInputSourceImplementation::readRun();
    case AccessState::SEEKING_RUN:
      accessState_.setState(AccessState::SEEKING_SUBRUN);
      return runPrincipal();
    default:
      throw Exception(errors::LogicError)
        << "RootInputSource::readRun encountered an "
           "unknown or inappropriate AccessState.\n";
  }
}

std::unique_ptr<RunPrincipal>
RootInput::readRun_()
{
  return primaryFileSequence_->readRun_();
}

std::unique_ptr<RangeSetHandler>
RootInput::runRangeSetHandler()
{
  return primaryFileSequence_->runRangeSetHandler();
}

std::unique_ptr<SubRunPrincipal>
RootInput::readSubRun(cet::exempt_ptr<RunPrincipal const> rp)
{
  switch (accessState_.state()) {
    case AccessState::SEQUENTIAL:
      return DecrepitRelicInputSourceImplementation::readSubRun(rp);
    case AccessState::SEEKING_SUBRUN:
      accessState_.setState(AccessState::SEEKING_EVENT);
      return subRunPrincipal();
    default:
      throw Exception(errors::LogicError)
        << "RootInputSource::readSubRun encountered an "
           "unknown or inappropriate AccessState.\n";
  }
}

std::unique_ptr<SubRunPrincipal>
RootInput::readSubRun_()
{
  return primaryFileSequence_->readSubRun_(runPrincipalExemptPtr());
}

std::unique_ptr<RangeSetHandler>
RootInput::subRunRangeSetHandler()
{
  return primaryFileSequence_->subRunRangeSetHandler();
}

std::unique_ptr<EventPrincipal>
RootInput::readEvent(cet::exempt_ptr<SubRunPrincipal const> srp)
{
  return readEvent_(srp);
}

std::unique_ptr<EventPrincipal>
RootInput::readEvent_(cet::exempt_ptr<SubRunPrincipal const> srp)
{
  switch (accessState_.state()) {
    case AccessState::SEQUENTIAL:
      return DecrepitRelicInputSourceImplementation::readEvent(srp);
    case AccessState::SEEKING_EVENT:
      accessState_.resetState();
      return eventPrincipal();
    default:
      throw Exception(errors::LogicError)
        << "RootInputSource::readEvent encountered an "
           "unknown or inappropriate AccessState.\n";
  }
}

std::unique_ptr<EventPrincipal>
RootInput::readEvent_()
{
  std::unique_ptr<EventPrincipal> result;
  if (!result.get()) {
    result = primaryFileSequence_->readEvent_();
  }
  if (result.get()) {
    accessState_.setLastReadEventID(result->id());
    accessState_.setRootFileForLastReadEvent(
      primaryFileSequence_->rootFileForLastReadEvent());
  }
  return result;
}

DEFINE_ART_INPUT_SOURCE(RootInput)
