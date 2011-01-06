// ======================================================================
//
// RootInput: This is an InputSource
//
// ======================================================================

#include "art/Framework/IO/Input/RootInput.h"

#include "TTreeCache.h"
#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/Core/RunPrincipal.h"
#include "art/Framework/Core/SubRunPrincipal.h"
#include "art/Framework/IO/Input/RootInputFileSequence.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Utilities/Exception.h"
#include <cassert>
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
{
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

std::auto_ptr<EventPrincipal>
  RootInput::readEvent_( )
{
  if (secondaryFileSequence_ && !branchIDsToReplace_[InEvent].empty()) {
    std::auto_ptr<EventPrincipal> primaryPrincipal = primaryFileSequence_->readEvent_();
    std::auto_ptr<EventPrincipal> secondaryPrincipal = secondaryFileSequence_->readIt(primaryPrincipal->id(), primaryPrincipal->subRun(), true);
    if (secondaryPrincipal.get() != 0) {
      checkConsistency(*primaryPrincipal, *secondaryPrincipal);
      primaryPrincipal->recombine(*secondaryPrincipal, branchIDsToReplace_[InEvent]);
    } else {
      throw art::Exception(errors::MismatchedInputFiles, "RootInput::readEvent_") <<
        primaryPrincipal->id() << " is not found in the secondary input files\n";
    }
    return primaryPrincipal;
  }
  EventSourceSentry(*this);
  return primaryFileSequence_->readEvent_();
}  // readEvent_()

std::auto_ptr<EventPrincipal>
  RootInput::readIt( EventID const & id )
{
  if (secondaryFileSequence_) {
    std::auto_ptr<EventPrincipal> primaryPrincipal = primaryFileSequence_->readIt(id);
    std::auto_ptr<EventPrincipal> secondaryPrincipal = secondaryFileSequence_->readIt(id, primaryPrincipal->subRun(), true);
    if (secondaryPrincipal.get() != 0) {
      checkConsistency(*primaryPrincipal, *secondaryPrincipal);
      primaryPrincipal->recombine(*secondaryPrincipal, branchIDsToReplace_[InEvent]);
    } else {
      throw art::Exception(errors::MismatchedInputFiles, "RootInput::readIt") <<
        primaryPrincipal->id() << " is not found in the secondary input files\n";
    }
    return primaryPrincipal;
  }
  EventSourceSentry(*this);
  return primaryFileSequence_->readIt(id);
}  // readIt()

InputSource::ItemType
  RootInput::getNextItemType()
{
  return primaryFileSequence_->getNextItemType();
}

// Rewind to before the first event that was read.
void
  RootInput::rewind_()
{
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
