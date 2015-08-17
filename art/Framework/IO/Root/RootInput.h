#ifndef art_Framework_IO_Root_RootInput_h
#define art_Framework_IO_Root_RootInput_h
// vim: set sw=2:

#include "art/Framework/Core/DecrepitRelicInputSourceImplementation.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/IO/Catalog/InputFileCatalog.h"
#include "art/Framework/IO/Root/Inputfwd.h"
#include "art/Framework/IO/Root/RootInputFileSequence.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <array>
#include <memory>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace art {

class MasterProductRegistry;

class RootInput : public DecrepitRelicInputSourceImplementation {
public:
  virtual ~RootInput();
  RootInput(fhicl::ParameterSet const&, InputSourceDescription&);
  using DecrepitRelicInputSourceImplementation::runPrincipal;
  // Find the requested event and set the system up
  // to read run and subRun records where appropriate. Note the
  // corresponding seekToEvent function must exist in
  // RootInputFileSequence to avoid a compile error.
  template<typename T> bool seekToEvent(T eventSpec, bool exact = false);
private:
  class AccessState {
  public:
    enum State {
      SEQUENTIAL = 0,
      SEEKING_FILE, // 1
      SEEKING_RUN, // 2
      SEEKING_SUBRUN, // 3
      SEEKING_EVENT // 4
    };
  public:
    AccessState();
    State state() const { return state_; }
    void setState(State state);
    void resetState() { state_ = SEQUENTIAL; }
    EventID const& lastReadEventID() const { return lastReadEventID_; }
    void setLastReadEventID(EventID const& eid);
    EventID const& wantedEventID() const { return wantedEventID_; }
    void setWantedEventID(EventID const& eid);
    std::shared_ptr<RootInputFile> rootFileForLastReadEvent() const {
      return rootFileForLastReadEvent_;
    }
    void setRootFileForLastReadEvent(std::shared_ptr<RootInputFile> const&);
  private:
    State state_;
    EventID lastReadEventID_;
    std::shared_ptr<RootInputFile> rootFileForLastReadEvent_;
    EventID wantedEventID_;
  }; // class AccessState
  typedef std::shared_ptr<RootInputFile> RootInputFileSharedPtr;
  typedef input::EntryNumber EntryNumber;
private:
  InputFileCatalog  catalog_;
  std::unique_ptr<RootInputFileSequence> primaryFileSequence_;
  std::array<std::vector<BranchID>, NumBranchTypes>  branchIDsToReplace_;
  AccessState accessState_;
  MasterProductRegistry& mpr_;
private:
  virtual input::ItemType nextItemType();
  using DecrepitRelicInputSourceImplementation::readEvent;
  virtual std::unique_ptr<EventPrincipal>
    readEvent(std::shared_ptr<SubRunPrincipal>);
  virtual std::unique_ptr<EventPrincipal> readEvent_();
  std::unique_ptr<EventPrincipal>
    readEvent_(std::shared_ptr<SubRunPrincipal>);
  virtual std::shared_ptr<SubRunPrincipal>
    readSubRun(std::shared_ptr<RunPrincipal>);
  virtual std::shared_ptr<SubRunPrincipal> readSubRun_();
  virtual std::vector<std::shared_ptr<SubRunPrincipal>>
    readSubRunFromSecondaryFiles_();
  virtual std::shared_ptr<RunPrincipal> readRun();
  virtual std::shared_ptr<RunPrincipal> readRun_();
  virtual std::vector<std::shared_ptr<RunPrincipal>>
    readRunFromSecondaryFiles_();
  virtual std::shared_ptr<FileBlock> readFile(MasterProductRegistry&);
  virtual std::shared_ptr<FileBlock> readFile_();
  virtual void closeFile_();
  virtual void endJob();
  virtual input::ItemType getNextItemType();
  virtual void rewind_();
  template<typename T>
    EventID
    postSeekChecks(
      EventID const& foundID, T eventSpec,
      typename
      std::enable_if<std::is_convertible<T, off_t>::value>::type* = 0);
  template<typename T>
    EventID
    postSeekChecks(
      EventID const& foundID, T eventSpec,
      typename
      std::enable_if<!std::is_convertible<T, off_t>::value >::type* = 0);
  //virtual void storeMPRforBrokenRandomAccess(MasterProductRegistry&);
}; // class RootInput

template<typename T>
bool
RootInput::
seekToEvent(T eventSpec, bool exact)
{
  if (accessState_.state()) {
    throw Exception(errors::LogicError)
        << "Attempted to initiate a random access seek "
        << "with one already in progress at state = "
        << accessState_.state()
        << ".\n";
  }
  EventID foundID = primaryFileSequence_->seekToEvent(eventSpec, exact);
  if (!foundID.isValid()) {
    return false;
  }
  foundID = postSeekChecks(foundID, eventSpec);
  accessState_.setWantedEventID(foundID);
  if (primaryFileSequence_->rootFile() !=
      accessState_.rootFileForLastReadEvent()) {
    accessState_.setState(AccessState::SEEKING_FILE);
  }
  else if (foundID.runID() != accessState_.lastReadEventID().runID()) {
    accessState_.setState(AccessState::SEEKING_RUN);
  }
  else if (foundID.subRunID() !=
           accessState_.lastReadEventID().subRunID()) {
    accessState_.setState(AccessState::SEEKING_SUBRUN);
  }
  else {
    accessState_.setState(AccessState::SEEKING_EVENT);
  }
  return true;
}

template<typename T>
EventID
RootInput::
postSeekChecks(EventID const& foundID, T eventspec,
               typename
               std::enable_if<std::is_convertible<T, off_t>::value>::type*)
{
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

template<typename T>
EventID
RootInput::
postSeekChecks(EventID const& foundID, T,
               typename
               std::enable_if<!std::is_convertible<T, off_t>::value >::type*)
{
  // Default implementation is NOP.
  return foundID;
}

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_IO_Root_RootInput_h */
