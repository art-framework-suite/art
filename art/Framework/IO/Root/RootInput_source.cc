// ======================================================================
//
// RootInput: This is an InputSource
//
// ======================================================================

#include "art/Framework/IO/Root/RootInput.h"

#include "TTreeCache.h"
#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/Core/RunPrincipal.h"
#include "art/Framework/Core/SubRunPrincipal.h"
#include "art/Framework/IO/Root/RootInputFileSequence.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Utilities/Exception.h"
#include <cassert>
#include <memory>
#include <set>

using namespace art;

namespace art {
  class EventID;
}

static void
  checkHistoryConsistency( Principal const & primary
                         , Principal const & secondary
                         )
 {
  ProcessHistory const & ph1 = primary.processHistory();
  ProcessHistory const & ph2 = secondary.processHistory();
  if (ph1 != ph2 && !isAncestor(ph2, ph1)) {
    throw art::Exception( errors::MismatchedInputFiles
                        , "RootInput::checkConsistency"
                        )
      << "The secondary file is not an ancestor of the primary file\n";
  }
}  // checkHistoryConsistency()

static void
  checkConsistency( EventPrincipal const & primary
                  , EventPrincipal const & secondary
                  )
{
  if (!isSameEvent(primary, secondary)) {
    throw art::Exception( errors::MismatchedInputFiles
                        , "RootInput::checkConsistency"
                        )
      << primary.id()
      << " has inconsistent EventAuxiliary data in the primary and secondary file\n";
  }
  checkHistoryConsistency(primary, secondary);
}  // checkConsistency()

static void
  checkConsistency( SubRunPrincipal const & primary
                  , SubRunPrincipal const & secondary
                  )
{
  if (primary.id() != secondary.id()) {
    throw art::Exception( errors::MismatchedInputFiles
                        , "RootInput::checkConsistency"
                        )
      << primary.id()
      << " has inconsistent SubRunAuxiliary data in the primary and secondary file\n";
  }
  checkHistoryConsistency(primary, secondary);
}  // checkConsistency()

static void
  checkConsistency( RunPrincipal const & primary
                  , RunPrincipal const & secondary
                  )
{
  if (primary.id() != secondary.id()) {
    throw art::Exception( errors::MismatchedInputFiles
                        , "RootInput::checkConsistency"
                        )
      << primary.id()
      << " has inconsistent RunAuxiliary data in the primary and secondary file\n";
  }
  checkHistoryConsistency(primary, secondary);
}  // checkConsistency()

// ----------------------------------------------------------------------

RootInput::RootInput( fhicl::ParameterSet const & pset
                      , InputSourceDescription const & desc
                      )
: EDInputSource     ( pset, desc )
, primaryFileSequence_  ( new RootInputFileSequence( pset
                                                   , *this
                                                   , catalog()
                                                   , primary()
                        )                          )
, secondaryFileSequence_( catalog(1).empty()
                          ? 0
                          : new RootInputFileSequence( pset
                                                     , *this
                                                     , catalog(1)
                                                     , false
                        )                            )
, branchIDsToReplace_   ( )
, accessState_() {
  if (secondaryFileSequence_) {
    boost::array<std::set<BranchID>, NumBranchTypes> idsToReplace;
    ProductRegistry::ProductList const & secondary = secondaryFileSequence_->fileProductRegistry().productList();
    ProductRegistry::ProductList const & primary = primaryFileSequence_->fileProductRegistry().productList();
    typedef ProductRegistry::ProductList::const_iterator const_iterator;
    for (const_iterator it = secondary.begin(), itEnd = secondary.end(); it != itEnd; ++it) {
      if (it->second.present()) idsToReplace[it->second.branchType()].insert(it->second.branchID());
    }
    for (const_iterator it = primary.begin(), itEnd = primary.end(); it != itEnd; ++it) {
      if (it->second.present()) idsToReplace[it->second.branchType()].erase(it->second.branchID());
    }
    if (idsToReplace[InEvent].empty() && idsToReplace[InSubRun].empty() && idsToReplace[InRun].empty()) {
      secondaryFileSequence_.reset();
    }
    else {
      for (int i = InEvent; i < NumBranchTypes; ++i) {
        branchIDsToReplace_[i].reserve(idsToReplace[i].size());
        for (std::set<BranchID>::const_iterator it = idsToReplace[i].begin(), itEnd = idsToReplace[i].end();
             it != itEnd; ++it) {
          branchIDsToReplace_[i].push_back(*it);
        }
      }
    }
  }
}

RootInput::~RootInput()
{ }

RootInput::AccessState::AccessState() :
      state_(SEQUENTIAL),
      lastReadEventID_(),
      rootFileForLastReadEvent_(),
      wantedEventID_()
    { }

void RootInput::AccessState::setState(State state) {
  state_ = state;
}

void RootInput::AccessState::setLastReadEventID(EventID const &eid) {
  lastReadEventID_ = eid;
}

void RootInput::AccessState::setWantedEventID(EventID const &eid) {
  wantedEventID_ = eid;
}

void
RootInput::AccessState::
setRootFileForLastReadEvent(boost::shared_ptr<RootInputFile> const &ptr) {
  rootFileForLastReadEvent_ = ptr; 
}

void
  RootInput::endJob()
{
  if (secondaryFileSequence_)
    secondaryFileSequence_->endJob();
  primaryFileSequence_->endJob();
}

boost::shared_ptr<FileBlock>
  RootInput::readFile_( )
{
  if (secondaryFileSequence_) {
      boost::shared_ptr<FileBlock> fb = primaryFileSequence_->readFile_();
      fb->setNotFastCopyable();
      return fb;
  }
  return primaryFileSequence_->readFile_();
}

void
  RootInput::closeFile_( )
{
  primaryFileSequence_->closeFile_();
}

boost::shared_ptr<RunPrincipal>
  RootInput::readRun_( )
{
  if (secondaryFileSequence_ && !branchIDsToReplace_[InRun].empty()) {
    boost::shared_ptr<RunPrincipal> primaryPrincipal = primaryFileSequence_->readRun_();
    boost::shared_ptr<RunPrincipal> secondaryPrincipal = secondaryFileSequence_->readIt(primaryPrincipal->id());
    if (secondaryPrincipal.get() != 0) {
      checkConsistency(*primaryPrincipal, *secondaryPrincipal);
      primaryPrincipal->recombine(*secondaryPrincipal, branchIDsToReplace_[InRun]);
    } else {
      throw art::Exception(errors::MismatchedInputFiles, "RootInput::readRun_")
        << " Run " << primaryPrincipal->run()
        << " is not found in the secondary input files\n";
    }
    return primaryPrincipal;
  }
  return primaryFileSequence_->readRun_();
}  // readRun_()

boost::shared_ptr<SubRunPrincipal>
  RootInput::readSubRun_( )
{
  if (secondaryFileSequence_ && !branchIDsToReplace_[InSubRun].empty()) {
    boost::shared_ptr<SubRunPrincipal> primaryPrincipal = primaryFileSequence_->readSubRun_();
    boost::shared_ptr<SubRunPrincipal> secondaryPrincipal = secondaryFileSequence_->readIt(primaryPrincipal->id());
    if (secondaryPrincipal.get() != 0) {
      checkConsistency(*primaryPrincipal, *secondaryPrincipal);
      primaryPrincipal->recombine(*secondaryPrincipal, branchIDsToReplace_[InSubRun]);
    } else {
      throw art::Exception(errors::MismatchedInputFiles, "RootInput::readSubRun_")
        << " Run " << primaryPrincipal->run()
        << " SubRun " << primaryPrincipal->subRun()
        << " is not found in the secondary input files\n";
    }
    return primaryPrincipal;
  }
  return primaryFileSequence_->readSubRun_();
}  // readSubRun_()

input::ItemType
RootInput::nextItemType() {
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
      << "RootInputSource::nextItemType encountered an unknown AccessState.\n";
  }
}

std::auto_ptr<EventPrincipal>
  RootInput::readEvent(boost::shared_ptr<SubRunPrincipal> srp)
{
  switch (accessState_.state()) {
  case AccessState::SEQUENTIAL:
    return DecrepitRelicInputSourceImplementation::readEvent(srp);
  case AccessState::SEEKING_EVENT:
    accessState_.resetState();
    {
      std::auto_ptr<EventPrincipal> result;
      if (secondaryFileSequence_) {
        std::auto_ptr<EventPrincipal> primaryPrincipal =
          primaryFileSequence_->readIt(accessState_.wantedEventID(), true);
        std::auto_ptr<EventPrincipal> secondaryPrincipal =
          secondaryFileSequence_->readIt(accessState_.wantedEventID(), true);
        if (secondaryPrincipal.get() != 0) {
          checkConsistency(*primaryPrincipal, *secondaryPrincipal);
          primaryPrincipal->recombine(*secondaryPrincipal, branchIDsToReplace_[InEvent]);
        } else {
          throw art::Exception(errors::MismatchedInputFiles, "RootInput::readIt") <<
            primaryPrincipal->id() << " is not found in the secondary input files\n";
        }
        result = primaryPrincipal;
      }
      if (!result.get()) result = primaryFileSequence_->readIt(accessState_.wantedEventID(),true);
      if (result.get()) {
        accessState_.setLastReadEventID(result->id());
        accessState_.setRootFileForLastReadEvent(primaryFileSequence_->rootFileForLastReadEvent());
      }
      return result;
    }
  default:
    throw Exception(errors::LogicError)
      << "RootInputSource::readEvent encountered an unknown or inappropriate AccessState.\n";
  }
}

boost::shared_ptr<SubRunPrincipal>
  RootInput::readSubRun(boost::shared_ptr<RunPrincipal> rp)
{
  switch (accessState_.state()) {
  case AccessState::SEQUENTIAL:
    return DecrepitRelicInputSourceImplementation::readSubRun(rp);
  case AccessState::SEEKING_SUBRUN:
    accessState_.setState(AccessState::SEEKING_EVENT);
    if (secondaryFileSequence_ && !branchIDsToReplace_[InSubRun].empty()) {
      boost::shared_ptr<SubRunPrincipal> primaryPrincipal =
        primaryFileSequence_->readIt(accessState_.wantedEventID().subRunID());
        boost::shared_ptr<SubRunPrincipal> secondaryPrincipal =
          secondaryFileSequence_->readIt(primaryPrincipal->id());
      if (secondaryPrincipal.get() != 0) {
        checkConsistency(*primaryPrincipal, *secondaryPrincipal);
        primaryPrincipal->recombine(*secondaryPrincipal, branchIDsToReplace_[InSubRun]);
      } else {
        throw art::Exception(errors::MismatchedInputFiles, "RootInput::readSubRun_")
          << " Run " << primaryPrincipal->run()
          << " SubRun " << primaryPrincipal->subRun()
          << " is not found in the secondary input files\n";
      }
      setSubRunPrincipal(primaryPrincipal);
      return primaryPrincipal;
    }
    setSubRunPrincipal(primaryFileSequence_->readSubRun_());
    return subRunPrincipal();
  default:
    throw Exception(errors::LogicError)
      << "RootInputSource::readSubRun encountered an unknown or inappropriate AccessState.\n";
  }
}

boost::shared_ptr<RunPrincipal>
  RootInput::readRun()
{
  switch (accessState_.state()) {
  case AccessState::SEQUENTIAL:
    return DecrepitRelicInputSourceImplementation::readRun();
  case AccessState::SEEKING_RUN:
    accessState_.setState(AccessState::SEEKING_SUBRUN);
    if (secondaryFileSequence_ && !branchIDsToReplace_[InRun].empty()) {
      boost::shared_ptr<RunPrincipal> primaryPrincipal = primaryFileSequence_->readIt(accessState_.wantedEventID().runID());
      boost::shared_ptr<RunPrincipal> secondaryPrincipal = secondaryFileSequence_->readIt(primaryPrincipal->id());
      if (secondaryPrincipal.get() != 0) {
        checkConsistency(*primaryPrincipal, *secondaryPrincipal);
        primaryPrincipal->recombine(*secondaryPrincipal, branchIDsToReplace_[InRun]);
      } else {
        throw art::Exception(errors::MismatchedInputFiles, "RootInput::readRun_")
          << " Run " << primaryPrincipal->run()
          << " is not found in the secondary input files\n";
      }
      setRunPrincipal(primaryPrincipal);
      return primaryPrincipal;
    }
    setRunPrincipal(primaryFileSequence_->readIt(accessState_.wantedEventID().runID()));
    return runPrincipal();
  default:
    throw Exception(errors::LogicError)
      << "RootInputSource::readRun encountered an unknown or inappropriate AccessState.\n";
  }
}

boost::shared_ptr<FileBlock>
RootInput::readFile() {
  switch (accessState_.state()) {
  case AccessState::SEQUENTIAL:
    return DecrepitRelicInputSourceImplementation::readFile();
  case AccessState::SEEKING_FILE:
    accessState_.setState(AccessState::SEEKING_RUN);
    return readFile_();
  default:
    throw Exception(errors::LogicError)
      << "RootInputSource::readFile encountered an unknown or inappropriate AccessState.\n";
  }  
}

std::auto_ptr<EventPrincipal>
  RootInput::readEvent_( )
{
  std::auto_ptr<EventPrincipal> result;
  if (secondaryFileSequence_ && !branchIDsToReplace_[InEvent].empty()) {
    std::auto_ptr<EventPrincipal> primaryPrincipal = primaryFileSequence_->readEvent_();
    std::auto_ptr<EventPrincipal> secondaryPrincipal =
       secondaryFileSequence_->readIt(primaryPrincipal->id(), true);
    if (secondaryPrincipal.get() != 0) {
      checkConsistency(*primaryPrincipal, *secondaryPrincipal);
      primaryPrincipal->recombine(*secondaryPrincipal, branchIDsToReplace_[InEvent]);
    } else {
      throw art::Exception(errors::MismatchedInputFiles, "RootInput::readEvent_") <<
        primaryPrincipal->id() << " is not found in the secondary input files\n";
    }
    result = primaryPrincipal;
  }
  if (!result.get()) result = primaryFileSequence_->readEvent_();
  if (result.get()) {
    accessState_.setLastReadEventID(result->id());
    accessState_.setRootFileForLastReadEvent(primaryFileSequence_->rootFileForLastReadEvent());
  }
  return result;
}  // readEvent_()

std::auto_ptr<EventPrincipal>
  RootInput::readIt( EventID const & id )
{
  assert("SHOULD NOT BE CALLED" == 0);
  std::auto_ptr<EventPrincipal> result;
  if (secondaryFileSequence_) {
    std::auto_ptr<EventPrincipal> primaryPrincipal = primaryFileSequence_->readIt(id);
    std::auto_ptr<EventPrincipal> secondaryPrincipal =
       secondaryFileSequence_->readIt(id, true);
    if (secondaryPrincipal.get() != 0) {
      checkConsistency(*primaryPrincipal, *secondaryPrincipal);
      primaryPrincipal->recombine(*secondaryPrincipal, branchIDsToReplace_[InEvent]);
    } else {
      throw art::Exception(errors::MismatchedInputFiles, "RootInput::readIt") <<
        primaryPrincipal->id() << " is not found in the secondary input files\n";
    }
    result = primaryPrincipal;
  }
  if (!result.get()) result = primaryFileSequence_->readIt(id);
  if (result.get()) {
    accessState_.setLastReadEventID(result->id());
    accessState_.setRootFileForLastReadEvent(primaryFileSequence_->rootFileForLastReadEvent());
  }
}  // readIt()

input::ItemType
  RootInput::getNextItemType()
{
  return primaryFileSequence_->getNextItemType();
}

// Rewind to before the first event that was read.
void
  RootInput::rewind_()
{
  accessState_.resetState();
  primaryFileSequence_->rewind_();
}

// Advance "offset" events.  Offset can be positive or negative (or zero).
void
RootInput::skip(int offset)
{
  primaryFileSequence_->skip(offset);
}

// ======================================================================

DEFINE_ART_INPUT_SOURCE(RootInput);

// ======================================================================
