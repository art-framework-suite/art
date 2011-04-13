#ifndef art_Framework_IO_Root_RootInput_h
#define art_Framework_IO_Root_RootInput_h

// ======================================================================
//
// RootInput: This is an InputSource
//
// ======================================================================

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/IO/Root/Inputfwd.h"
#include "art/Framework/IO/Root/RootInputFileSequence.h"
#include "art/Framework/IO/Catalog/InputFileCatalog.h"
#include "art/Framework/Core/DecrepitRelicInputSourceImplementation.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "boost/array.hpp"
#include "boost/scoped_ptr.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/type_traits.hpp"
#include "boost/utility.hpp"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>
#include <string>
#include <vector>

// ----------------------------------------------------------------------


class art::RootInput : public art::DecrepitRelicInputSourceImplementation
{
public:
  explicit RootInput(fhicl::ParameterSet    const & pset,
                     InputSourceDescription const & desc);
  virtual ~RootInput( );

  using DecrepitRelicInputSourceImplementation::productRegistryUpdate;
  using DecrepitRelicInputSourceImplementation::runPrincipal;

  // This function will find the requested event and set the system up
  // to read run and subRrun records where appropriate. Note the
  // corresponding seekToEvent function must exist in
  // RootInputFileSequence to avoid a compile error.
  template <typename T> bool seekToEvent(T eventSpec, bool exact = false);

private:
  InputFileCatalog  catalog_;
  boost::scoped_ptr< RootInputFileSequence >             primaryFileSequence_;
  boost::array< std::vector<BranchID>, NumBranchTypes >  branchIDsToReplace_;

  class AccessState {
  public:
    enum State {
      SEQUENTIAL = 0,
      SEEKING_FILE,
      SEEKING_RUN,
      SEEKING_SUBRUN,
      SEEKING_EVENT
    };

    AccessState();

    State state() const { return state_; }

    void setState(State state);

    void resetState() {
      state_ = SEQUENTIAL;
    }

    EventID const &lastReadEventID() const { return lastReadEventID_; }

    void setLastReadEventID(EventID const &eid);

    EventID const &wantedEventID() const { return wantedEventID_; }

    void setWantedEventID(EventID const &eid);

    boost::shared_ptr<RootInputFile> rootFileForLastReadEvent() const {
      return rootFileForLastReadEvent_;
    }

    void setRootFileForLastReadEvent(boost::shared_ptr<RootInputFile> const
                                     &ptr);

  private:
    State state_;
    EventID lastReadEventID_;
    boost::shared_ptr<RootInputFile> rootFileForLastReadEvent_;
    EventID wantedEventID_;
  };

  AccessState accessState_;

  typedef  boost::shared_ptr<RootInputFile>  RootInputFileSharedPtr;
  typedef  input::EntryNumber           EntryNumber;

  virtual input::ItemType nextItemType();
  virtual std::auto_ptr<EventPrincipal>
  readEvent(boost::shared_ptr<SubRunPrincipal> srp);
  virtual boost::shared_ptr<SubRunPrincipal>
  readSubRun(boost::shared_ptr<RunPrincipal> rp);
  virtual boost::shared_ptr<RunPrincipal>
  readRun();
  virtual boost::shared_ptr<FileBlock>
  readFile();
  virtual std::auto_ptr<EventPrincipal>
  readEvent_( );
  virtual boost::shared_ptr<SubRunPrincipal>
  readSubRun_( );
  virtual boost::shared_ptr<RunPrincipal>
  readRun_( );
  virtual boost::shared_ptr<FileBlock>
  readFile_( );
  virtual void
  closeFile_( );
  virtual void
  endJob( );
  virtual input::ItemType
  getNextItemType( );
  virtual std::auto_ptr<EventPrincipal>
  readIt( EventID const & id );
  virtual void
  skip( int offset );
  virtual void
  rewind_( );

  template <typename T> 
  EventID postSeekChecks(EventID const &foundID, T eventSpec,
                         typename boost::enable_if<boost::is_convertible<T, off_t> >::type *dummy = 0);

  template <typename T>
  EventID postSeekChecks(EventID const &foundID, T eventSpec,
                         typename boost::disable_if<boost::is_convertible<T, off_t> >::type *dummy = 0);
}; // RootInput

template <typename T>
bool art::RootInput::seekToEvent(T eventSpec, bool exact) {
  if (accessState_.state()) {
    throw Exception(errors::LogicError)
      << "Attempted to initiate a random access seek "
      << "with one already in progress at state = "
      << accessState_.state()
      << ".\n";
  }
  EventID foundID = primaryFileSequence_->seekToEvent(eventSpec, exact);
  if (!foundID.isValid()) return false;
  foundID = postSeekChecks(foundID, eventSpec);
  accessState_.setWantedEventID(foundID);
  if (primaryFileSequence_->rootFile() !=
      accessState_.rootFileForLastReadEvent()) {
    accessState_.setState(AccessState::SEEKING_FILE);
  } else if (foundID.runID() != accessState_.lastReadEventID().runID()) {
    accessState_.setState(AccessState::SEEKING_RUN);
  } else if (foundID.subRunID() !=
             accessState_.lastReadEventID().subRunID()) {
    accessState_.setState(AccessState::SEEKING_SUBRUN);
  } else {
    accessState_.setState(AccessState::SEEKING_EVENT);
  }
  return true;
}

template <typename T>
art::EventID 
art::RootInput::postSeekChecks(EventID const &foundID,
                               T eventspec,
                               typename boost::enable_if<boost::is_convertible<T, off_t> >::type *) {
  if (eventspec == 0 && foundID == accessState_.lastReadEventID()) {
    // We're supposed to be reading the, "next" event but it's a
    // duplicate of the current one: skip it.
    mf::LogWarning("DuplicateEvent")
      << "Duplicate Events found: "
      << "both events were " << foundID << ".\n"
      << "The duplicate will be skipped.\n";
    return primaryFileSequence_->seekToEvent(1, false);
  }
  return foundID;
}

template <typename T>
art::EventID
art::RootInput::postSeekChecks(EventID const &foundID,
                               T eventspec,
                               typename boost::disable_if<boost::is_convertible<T, off_t> >::type *) {
  // Default implementation is NOP.
  return foundID;
}

// ======================================================================

#endif /* art_Framework_IO_Root_RootInput_h */

// Local Variables:
// mode: c++
// End:
