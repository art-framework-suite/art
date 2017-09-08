#ifndef art_Framework_IO_Root_RootInput_h
#define art_Framework_IO_Root_RootInput_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/DecrepitRelicInputSourceImplementation.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/IO/Catalog/InputFileCatalog.h"
#include "art/Framework/IO/Root/Inputfwd.h"
#include "art/Framework/IO/Root/RootInputFileSequence.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "fhiclcpp/types/ConfigurationTable.h"
#include "fhiclcpp/types/KeysToIgnore.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/TableFragment.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <array>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace art {

  class MasterProductRegistry;

  class RootInput final : public DecrepitRelicInputSourceImplementation {

  public: // CONFIGURATION

    struct Config {

      fhicl::Atom<std::string> module_type{fhicl::Name("module_type")};
      fhicl::TableFragment<DecrepitRelicInputSourceImplementation::Config> drisi_config;
      fhicl::TableFragment<InputFileCatalog::Config> ifc_config;
      fhicl::TableFragment<RootInputFileSequence::Config> rifs_config;

      struct KeysToIgnore {
        std::set<std::string>
        operator()() const
        {
          return {"module_label"};
        }

      };

    };

    using Parameters = fhicl::WrappedTable<Config, Config::KeysToIgnore>;

  private: // TYPES

    class AccessState {

    public: // TYPES

      enum State {
        SEQUENTIAL = 0,
        SEEKING_FILE, // 1
        SEEKING_RUN, // 2
        SEEKING_SUBRUN, // 3
        SEEKING_EVENT // 4
      };

    public:

      ~AccessState();

      AccessState();

      State
      state() const;

      void
      setState(State state);

      void
      resetState();

      EventID const&
      lastReadEventID() const;

      void
      setLastReadEventID(EventID const&);

      EventID const&
      wantedEventID() const;

      void
      setWantedEventID(EventID const&);

      std::shared_ptr<RootInputFile>
      rootFileForLastReadEvent() const;

      void
      setRootFileForLastReadEvent(std::shared_ptr<RootInputFile> const&);

    private:

      State state_;
      EventID lastReadEventID_;
      std::shared_ptr<RootInputFile> rootFileForLastReadEvent_;
      EventID wantedEventID_;

    };

    typedef input::EntryNumber EntryNumber;

  public: // MEMBER FUNCTIONS -- Special Member Functions

    ~RootInput();

    RootInput(Parameters const&, InputSourceDescription&);

    RootInput(RootInput const&) = delete;

    RootInput(RootInput&&) = delete;

    RootInput&
    operator=(RootInput const&) = delete;

    RootInput&
    operator=(RootInput&&) = delete;

  public:

    // Find the requested event and set the system up to read run and
    // subRun records where appropriate. Note the corresponding
    // seekToEvent function must exist in RootInputFileSequence to
    // avoid a compile error.
    template<typename T>
    bool
    seekToEvent(T eventSpec, bool exact = false);

  private: // MEMBER FUNCTIONS -- Serial Interface

    virtual
    input::ItemType
    nextItemType() override;

    virtual
    std::unique_ptr<FileBlock>
    readFile(MasterProductRegistry&) override;

    // Not Implemented
    //virtual
    //void
    //closeFile() = 0;

    virtual
    std::unique_ptr<RunPrincipal>
    readRun() override;

    virtual
    std::unique_ptr<SubRunPrincipal>
    readSubRun(cet::exempt_ptr<RunPrincipal>) override;

    virtual
    std::unique_ptr<EventPrincipal>
    readEvent(cet::exempt_ptr<SubRunPrincipal>) override;

    virtual
    std::unique_ptr<RangeSetHandler>
    runRangeSetHandler() override;

    virtual
    std::unique_ptr<RangeSetHandler>
    subRunRangeSetHandler() override;

  private: // MEMBER FUNCTIONS -- Job Interface

    // Not Implemented.
    //virtual
    //void
    //doBeginJob() override;

    virtual
    void
    endJob() override;

  private: // MEMBER FUNCTIONS -- Random Access Interface

    // Note: This pulls in:
    //      virtual
    //      std::unique_ptr<EventPrincipal>
    //      readEvent(EventID const&) override;
    using DecrepitRelicInputSourceImplementation::readEvent;

    // Not Implemented.
    //virtual
    //void
    //skipEvents(int n);

    // Not Implemented.
    //virtual
    //void
    //rewind() override;

  private: // MEMBER FUNCTIONS -- Required by DecrepitRelicInputSourceImplementation

    virtual
    input::ItemType
    getNextItemType() override;

    virtual
    std::unique_ptr<RunPrincipal>
    readRun_() override;

    virtual
    std::unique_ptr<SubRunPrincipal>
    readSubRun_(cet::exempt_ptr<RunPrincipal>) override;

    virtual
    std::unique_ptr<EventPrincipal>
    readEvent_() override;

  private: // MEMBER FUNCTIONS -- DecrepitRelicInputSourceImplementation Interface

    virtual
    std::unique_ptr<FileBlock>
    readFile_() override;

    virtual
    void
    closeFile_() override;

    virtual
    void
    rewind_() override;

  private: // MEMBER FUNCTIONS

    std::unique_ptr<EventPrincipal>
    readEvent_(cet::exempt_ptr<SubRunPrincipal>);

  private:

    template<typename T>
    std::enable_if_t<std::is_convertible<T, off_t>::value, EventID>
    postSeekChecks(EventID const& foundID, T eventSpec);

    template<typename T>
    std::enable_if_t<!std::is_convertible<T, off_t>::value, EventID>
    postSeekChecks(EventID const& foundID, T eventSpec);

  private:

    InputFileCatalog catalog_;
    std::unique_ptr<RootInputFileSequence> primaryFileSequence_;
    AccessState accessState_;
    MasterProductRegistry& mpr_;

  };

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
  std::enable_if_t < !std::is_convertible<T, off_t>::value, EventID >
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
