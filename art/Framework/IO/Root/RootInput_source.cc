#include "art/Framework/IO/Root/RootInput.h"
// vim: set sw=2:

#include "TTreeCache.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/IO/Root/FastCloningInfoProvider.h"
#include "art/Framework/IO/Root/RootInputFileSequence.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Utilities/Exception.h"
#include "cetlib/make_unique.h"
#include <cassert>
#include <memory>
#include <set>

using namespace art;

RootInput::
AccessState::
AccessState() :
  state_(SEQUENTIAL),
  lastReadEventID_(),
  rootFileForLastReadEvent_(),
  wantedEventID_()
{
}

void
RootInput::
AccessState::
setState(State state)
{
  state_ = state;
}

void
RootInput::
AccessState::
setLastReadEventID(EventID const& eid)
{
  lastReadEventID_ = eid;
}

void
RootInput::
AccessState::
setWantedEventID(EventID const& eid)
{
  wantedEventID_ = eid;
}

void
RootInput::
AccessState::
setRootFileForLastReadEvent(std::shared_ptr<RootInputFile> const& ptr)
{
  rootFileForLastReadEvent_ = ptr;
}

RootInput::~RootInput()
{
}

RootInput::
RootInput(fhicl::ParameterSet const& pset, InputSourceDescription& desc)
  : DecrepitRelicInputSourceImplementation(pset, desc)
  , catalog_(pset)
  , primaryFileSequence_(cet::make_unique<RootInputFileSequence>(pset,
    catalog_, FastCloningInfoProvider(cet::exempt_ptr<RootInput>(this)),
    processingMode(), desc.productRegistry, processConfiguration()))
  , branchIDsToReplace_()
  , accessState_()
  , mpr_(desc.productRegistry)
{
}

void
RootInput::
endJob()
{
  primaryFileSequence_->endJob();
}

void
RootInput::
closeFile_()
{
  primaryFileSequence_->closeFile_();
}

void
RootInput::
rewind_()
{
  // Rewind to before the first event that was read.
  accessState_.resetState();
  primaryFileSequence_->rewind_();
}

input::ItemType
RootInput::
getNextItemType()
{
  return primaryFileSequence_->getNextItemType();
}

input::ItemType
RootInput::
nextItemType()
{
  switch (accessState_.state()) {
    case AccessState::SEQUENTIAL:
      return DecrepitRelicInputSourceImplementation::nextItemType();
    case AccessState::SEEKING_FILE:
      return input::IsFile;
    case AccessState::SEEKING_RUN:
      return input::IsRun;
    case AccessState::SEEKING_SUBRUN:
      return input::IsSubRun;
    case AccessState::SEEKING_EVENT:
      return input::IsEvent;
    default:
      throw Exception(errors::LogicError)
          << "RootInputSource::nextItemType encountered an "
             "unknown AccessState.\n";
  }
}

std::shared_ptr<FileBlock>
RootInput::
readFile(MasterProductRegistry& /*mpr*/)
{
  switch (accessState_.state()) {
    case AccessState::SEQUENTIAL:
      return DecrepitRelicInputSourceImplementation::readFile(mpr_);
    case AccessState::SEEKING_FILE:
      accessState_.setState(AccessState::SEEKING_RUN);
      return readFile_();
    default:
      throw Exception(errors::LogicError)
          << "RootInputSource::readFile encountered an "
             "unknown or inappropriate AccessState.\n";
  }
}

std::shared_ptr<FileBlock>
RootInput::
readFile_()
{
  return primaryFileSequence_->readFile_();
}

std::shared_ptr<RunPrincipal>
RootInput::
readRun()
{
  switch (accessState_.state()) {
    case AccessState::SEQUENTIAL:
      return DecrepitRelicInputSourceImplementation::readRun();
    case AccessState::SEEKING_RUN:
      accessState_.setState(AccessState::SEEKING_SUBRUN);
      setRunPrincipal(primaryFileSequence_->readIt(
        accessState_.wantedEventID().runID()));
      return runPrincipal();
    default:
      throw Exception(errors::LogicError)
          << "RootInputSource::readRun encountered an "
             "unknown or inappropriate AccessState.\n";
  }
}

std::shared_ptr<RunPrincipal>
RootInput::
readRun_()
{
  return primaryFileSequence_->readRun_();
}

std::vector<std::shared_ptr<RunPrincipal>>
RootInput::
readRunFromSecondaryFiles_()
{
  return std::move(primaryFileSequence_->readRunFromSecondaryFiles_());
}

std::shared_ptr<SubRunPrincipal>
RootInput::
readSubRun(std::shared_ptr<RunPrincipal> rp)
{
  switch (accessState_.state()) {
    case AccessState::SEQUENTIAL:
      return DecrepitRelicInputSourceImplementation::readSubRun(rp);
    case AccessState::SEEKING_SUBRUN:
      accessState_.setState(AccessState::SEEKING_EVENT);
      {
        std::shared_ptr<SubRunPrincipal> srp(primaryFileSequence_->readIt(
          accessState_.wantedEventID().subRunID(), rp));
        srp->setRunPrincipal(rp);
        setSubRunPrincipal(srp);
        return std::move(srp);
      }
    default:
      throw Exception(errors::LogicError)
          << "RootInputSource::readSubRun encountered an "
             "unknown or inappropriate AccessState.\n";
  }
}

std::shared_ptr<SubRunPrincipal>
RootInput::
readSubRun_()
{
  return primaryFileSequence_->readSubRun_(runPrincipal());
}

std::vector<std::shared_ptr<SubRunPrincipal>>
RootInput::
readSubRunFromSecondaryFiles_()
{
  return std::move(primaryFileSequence_->readSubRunFromSecondaryFiles_(runPrincipal()));
}

std::unique_ptr<EventPrincipal>
RootInput::
readEvent(std::shared_ptr<SubRunPrincipal> srp)
{
  return readEvent_(srp);
}

std::unique_ptr<EventPrincipal>
RootInput::
readEvent_(std::shared_ptr<SubRunPrincipal> srp)
{
  switch (accessState_.state()) {
    case AccessState::SEQUENTIAL:
      return DecrepitRelicInputSourceImplementation::readEvent(srp);
    case AccessState::SEEKING_EVENT:
      accessState_.resetState();
      {
        std::unique_ptr<EventPrincipal>
        result(primaryFileSequence_->readIt(accessState_.wantedEventID(),
                                            true));
        if (result.get()) {
          accessState_.setLastReadEventID(result->id());
          accessState_.setRootFileForLastReadEvent(
            primaryFileSequence_->rootFileForLastReadEvent());
          result->setSubRunPrincipal(srp);
        }
        return result;
      }
    default:
      throw Exception(errors::LogicError)
          << "RootInputSource::readEvent encountered an "
             "unknown or inappropriate AccessState.\n";
  }
}

std::unique_ptr<EventPrincipal>
RootInput::
readEvent_()
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

//void
//RootInput::
//storeMPRforBrokenRandomAccess(MasterProductRegistry& mpr)
//{
//  mpr_.reset(&mpr);
//}

//void
//RootInput::
//checkMPR(MasterProductRegistry const& mpr) const
//{
//  if (&mpr != mpr_.get()) {
//    throw Exception(errors::LogicError)
//        << "Consistency error: MasterProductRegistry in current call "
//           "does not match\n"
//        << "that stored!\n";
//  }
//}

DEFINE_ART_INPUT_SOURCE(RootInput)

