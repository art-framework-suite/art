#ifndef art_Framework_IO_Root_RootInput_h
#define art_Framework_IO_Root_RootInput_h
// vim: set sw=2:

#include "art/Framework/Core/DecrepitRelicInputSourceImplementation.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/IO/Catalog/InputFileCatalog.h"
#include "art/Framework/IO/Root/Inputfwd.h"
#include "art/Framework/IO/Root/RootInputFileSequence.h"
#include "art/Utilities/ConfigurationTable.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchID.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "fhiclcpp/types/KeysToIgnore.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/TableFragment.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <array>
#include <memory>
#include <string>
#include <vector>
#include <type_traits>

namespace art {

  class MasterProductRegistry;

  class RootInput final : public DecrepitRelicInputSourceImplementation {
  public:

    struct Config {
      fhicl::Atom<std::string> module_type { fhicl::Name("module_type") };
      fhicl::TableFragment<DecrepitRelicInputSourceImplementation::Config> drisi_config;
      fhicl::TableFragment<InputFileCatalog::Config> ifc_config;
      fhicl::TableFragment<RootInputFileSequence::Config> rifs_config;

      struct KeysToIgnore {
        std::set<std::string> operator()() const { return {"module_label"}; }
      };
    };

    using Parameters = art::WrappedTable<Config, Config::KeysToIgnore>;

    RootInput(Parameters const&, InputSourceDescription&);
    using DecrepitRelicInputSourceImplementation::runPrincipal;
    // Find the requested event and set the system up to read run and
    // subRun records where appropriate. Note the corresponding
    // seekToEvent function must exist in RootInputFileSequence to
    // avoid a compile error.
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
    typedef input::EntryNumber EntryNumber;
  private:
    InputFileCatalog  catalog_;
    std::unique_ptr<RootInputFileSequence> primaryFileSequence_;
    std::array<std::vector<BranchID>, NumBranchTypes>  branchIDsToReplace_;
    AccessState accessState_;
    MasterProductRegistry& mpr_;
  private:
    input::ItemType nextItemType() override;
    using DecrepitRelicInputSourceImplementation::readEvent;
    std::unique_ptr<EventPrincipal> readEvent(cet::exempt_ptr<SubRunPrincipal>) override;
    std::unique_ptr<EventPrincipal> readEvent_() override;
    std::unique_ptr<EventPrincipal> readEvent_(cet::exempt_ptr<SubRunPrincipal>);
    std::unique_ptr<SubRunPrincipal> readSubRun(cet::exempt_ptr<RunPrincipal>) override;
    std::unique_ptr<SubRunPrincipal> readSubRun_() override;
    std::unique_ptr<RunPrincipal> readRun() override;
    std::unique_ptr<RunPrincipal> readRun_() override;
    std::unique_ptr<FileBlock> readFile(MasterProductRegistry&) override;
    std::unique_ptr<FileBlock> readFile_() override;
    std::unique_ptr<RangeSetHandler> runRangeSetHandler() override;
    std::unique_ptr<RangeSetHandler> subRunRangeSetHandler() override;
    void closeFile_() override;
    void endJob() override;
    input::ItemType getNextItemType() override;
    void rewind_() override;

    template<typename T>
    std::enable_if_t<std::is_convertible<T, off_t>::value, EventID>
    postSeekChecks(EventID const& foundID, T eventSpec);

    template<typename T>
    std::enable_if_t<!std::is_convertible<T, off_t>::value, EventID>
    postSeekChecks(EventID const& foundID, T eventSpec);
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
    if (primaryFileSequence_->rootFile() != accessState_.rootFileForLastReadEvent()) {
      accessState_.setState(AccessState::SEEKING_FILE);
    }
    else if (foundID.runID() != accessState_.lastReadEventID().runID()) {
      accessState_.setState(AccessState::SEEKING_RUN);
    }
    else if (foundID.subRunID() != accessState_.lastReadEventID().subRunID()) {
      accessState_.setState(AccessState::SEEKING_SUBRUN);
    }
    else {
      accessState_.setState(AccessState::SEEKING_EVENT);
    }
    return true;
  }

  template<typename T>
  std::enable_if_t<std::is_convertible<T, off_t>::value, EventID>
  RootInput::
  postSeekChecks(EventID const& foundID, T eventspec)
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
  std::enable_if_t<!std::is_convertible<T, off_t>::value, EventID>
  RootInput::
  postSeekChecks(EventID const& foundID, T)
  {
    // Default implementation is NOP.
    return foundID;
  }

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_IO_Root_RootInput_h */
