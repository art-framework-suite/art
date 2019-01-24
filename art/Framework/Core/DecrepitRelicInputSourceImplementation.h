#ifndef art_Framework_Core_DecrepitRelicInputSourceImplementation_h
#define art_Framework_Core_DecrepitRelicInputSourceImplementation_h
// vim: set sw=2 expandtab :

//
//  This is the relic we inherited from CMS, which is not an interface.
//  The remaining comments are left over from that.
//
//  InputSource: Abstract interface for all input sources. Input
//  sources are responsible for creating an EventPrincipal, using data
//  controlled by the source, and external to the EventPrincipal itself.
//
//  The InputSource is also responsible for dealing with the "process
//  name list" contained within the EventPrincipal. Each InputSource has
//  to know what "process" (HLT, PROD, USER, USER1, etc.) the program is
//  part of. The InputSource is repsonsible for pushing this process name
//  onto the end of the process name list.
//
//  For now, we specify this process name to the constructor of the
//  InputSource. This should be improved.
//
//   Some questions about this remain:
//
//     1. What should happen if we "rerun" a process? i.e., if "USER1" is
//     already last in our input file, and we run again a job which claims
//     to be "USER1", what should happen? For now, we just quietly add
//     this to the history.
//
//     2. Do we need to detect a problem with a history like:
//           HLT PROD USER1 PROD
//     or is it up to the user not to do something silly? Right now, there
//     is no protection against such sillyness.
//
//  Some examples of InputSource subclasses may be:
//
//   1) EmptyEvent: creates EventPrincipals which contain no EDProducts.
//   2) RootInput: creates EventPrincipals which "contain" the data
//      read from a root file. This source should provide for delayed loading
//      of data, thus the quotation marks around contain.
//   3) DAQInputSource: creats EventPrincipals which contain raw data, as
//      delivered by the L1 trigger and event builder.
//

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Persistency/Provenance/Timestamp.h"
#include "cetlib/exempt_ptr.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/TableFragment.h"

#include <memory>
#include <string>

namespace art {

  class ActivityRegistry;

  class DecrepitRelicInputSourceImplementation : public InputSource {

  public: // CONFIGURATION
    struct Config {

      ~Config();
      Config();
      Config(Config const&) = delete;
      Config(Config&&) = delete;
      Config& operator=(Config const&) = delete;
      Config& operator=(Config&&) = delete;

      static char const* defaultMode();

      fhicl::Atom<int> maxEvents;
      fhicl::Atom<int> maxSubRuns;
      fhicl::Atom<int> reportFrequency;
      fhicl::Atom<bool> errorOnFailureToPut;
      fhicl::Atom<std::string> processingMode;
    };

  public: // MEMBER FUNCTIONS -- Special Member Functions
    DecrepitRelicInputSourceImplementation(fhicl::TableFragment<Config> const&,
                                           ModuleDescription const&);

    // Prevent concrete instances of this class even though we don't
    // have any (other) pure virtual functions.
    virtual ~DecrepitRelicInputSourceImplementation() noexcept = 0;

    DecrepitRelicInputSourceImplementation(
      DecrepitRelicInputSourceImplementation&&) = delete;

    DecrepitRelicInputSourceImplementation& operator=(
      DecrepitRelicInputSourceImplementation const&) = delete;

    DecrepitRelicInputSourceImplementation& operator=(
      DecrepitRelicInputSourceImplementation&&) = delete;

  public: // MEMBER FUNCTIONS -- Serial Access Interface
    // Inform subclasses that they're going to be told to close the file
    // for non-exceptional reasons (such as hitting the event limit).
    virtual void
    finish()
    {}

    virtual input::ItemType nextItemType() override;

    // Close current file
    virtual void closeFile() override;

    /// Read next file
    std::unique_ptr<FileBlock> readFile() override;

    // Read next run.
    virtual std::unique_ptr<RunPrincipal> readRun() override;

    // Read next subRun
    virtual std::unique_ptr<SubRunPrincipal> readSubRun(
      cet::exempt_ptr<RunPrincipal const>) override;

    // Read next event
    // Indicate inability to get a new event by returning a null unique_ptr.
    virtual std::unique_ptr<EventPrincipal> readEvent(
      cet::exempt_ptr<SubRunPrincipal const>) override;

    // Not implemented.
    // virtual
    // std::unique_ptr<RangeSetHandler>
    // runRangeSetHandler() = 0;

    // Not implemented.
    // virtual
    // std::unique_ptr<RangeSetHandler>
    // subRunRangeSetHandler() = 0;

  public: // MEMBER FUNCTIONS -- Job Interface
    // Called by framework at beginning of job
    virtual void doBeginJob() override;

    // Called by framework at end of job
    virtual void doEndJob() override;

  public: // MEMBER FUNCTIONS -- Random Access Interface
    // Skip the number of events specified.
    // Offset may be negative.
    virtual void skipEvents(int offset) override;

    // Begin again at the first event
    virtual void rewind() override;

  public: // MEMBER FUNCTIONS -- DecrepitRelicInputSourceImplementation specific
          // interface
    // RunsSubRunsAndEvents (default), RunsAndSubRuns, or Runs.
    ProcessingMode processingMode() const;

    // Accessor for maximum number of events to be read.
    // -1 is used for unlimited.
    int maxEvents() const;

    // Accessor for remaining number of events to be read.
    // -1 is used for unlimited.
    int remainingEvents() const;

    // Accessor for maximum number of subRuns to be read.
    // -1 is used for unlimited.
    int maxSubRuns() const;

    // Accessor for remaining number of subRuns to be read.
    // -1 is used for unlimited.
    int remainingSubRuns() const;

  public: // MEMBER FUNCTIONS -- DecrepitRelicInputSourceImplementation specific
          // interface
    // Reset the remaining number of events/subRuns to the maximum number.
    void repeat_();

    // issue an event report
    void issueReports(EventID const& eventID);

  protected: // MEMBER FUNCTIONS -- Utility Functions for Subclasses
    input::ItemType state() const;

    void setState(input::ItemType);

    void reset();

  private: // MEMBER FUNCTIONS -- Required Interface for Subclasses
    virtual input::ItemType getNextItemType() = 0;

    virtual std::unique_ptr<RunPrincipal> readRun_() = 0;

    virtual std::unique_ptr<SubRunPrincipal> readSubRun_(
      cet::exempt_ptr<RunPrincipal const>) = 0;

    virtual std::unique_ptr<EventPrincipal> readEvent_() = 0;

  private: // MEMBER FUNCTIONS -- Optional Serial Interface for Subclasses
    virtual std::unique_ptr<FileBlock> readFile_();

    virtual void closeFile_();

  private: // MEMBER FUNCTIONS -- Optional Job Interface for Subclasses
    virtual void beginJob();

    virtual void endJob();

  private
    : // MEMBER FUNCTIONS -- Optional Random Access Interface for Subclasses
    virtual void skip(int);

    virtual void rewind_();

  private: // MEMBER DATA
    ProcessingMode processingMode_{RunsSubRunsAndEvents};
    int maxEvents_;
    int maxSubRuns_;
    int const reportFrequency_;
    int remainingEvents_;
    int remainingSubRuns_;
    int numberOfEventsRead_{};
    input::ItemType state_{input::IsInvalid};
  };

} // namespace art

#endif /* art_Framework_Core_DecrepitRelicInputSourceImplementation_h */

// Local Variables:
// mode: c++
// End:
