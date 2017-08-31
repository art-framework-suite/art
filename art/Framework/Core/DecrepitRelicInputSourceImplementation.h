#ifndef art_Framework_Core_DecrepitRelicInputSourceImplementation_h
#define art_Framework_Core_DecrepitRelicInputSourceImplementation_h

/*----------------------------------------------------------------------

DecrepitRelicInputSourceImplementation: this is the relic we
inherited from CMS, which is not an interface. The remaining comments
are left over from that.

InputSource: Abstract interface for all input sources. Input
sources are responsible for creating an EventPrincipal, using data
controlled by the source, and external to the EventPrincipal itself.

The InputSource is also responsible for dealing with the "process
name list" contained within the EventPrincipal. Each InputSource has
to know what "process" (HLT, PROD, USER, USER1, etc.) the program is
part of. The InputSource is repsonsible for pushing this process name
onto the end of the process name list.

For now, we specify this process name to the constructor of the
InputSource. This should be improved.

 Some questions about this remain:

   1. What should happen if we "rerun" a process? i.e., if "USER1" is
   already last in our input file, and we run again a job which claims
   to be "USER1", what should happen? For now, we just quietly add
   this to the history.

   2. Do we need to detect a problem with a history like:
         HLT PROD USER1 PROD
   or is it up to the user not to do something silly? Right now, there
   is no protection against such sillyness.

Some examples of InputSource subclasses may be:

 1) EmptyEvent: creates EventPrincipals which contain no EDProducts.
 2) RootInput: creates EventPrincipals which "contain" the data
    read from a root file. This source should provide for delayed loading
    of data, thus the quotation marks around contain.
 3) DAQInputSource: creats EventPrincipals which contain raw data, as
    delivered by the L1 trigger and event builder.

----------------------------------------------------------------------*/

#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Persistency/Provenance/Timestamp.h"
#include "cetlib/exempt_ptr.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/TableFragment.h"

#include <memory>
#include <string>

// ----------------------------------------------------------------------

namespace art
{
  class ActivityRegistry;

  class DecrepitRelicInputSourceImplementation :
    public InputSource,
    private ProductRegistryHelper
  {
  public:


    DecrepitRelicInputSourceImplementation(DecrepitRelicInputSourceImplementation const&) = delete;
    DecrepitRelicInputSourceImplementation& operator=(DecrepitRelicInputSourceImplementation const&) = delete;

    struct Config {

      static constexpr char const* defaultMode() { return "RunsSubRunsAndEvents"; }

      fhicl::Atom<int> maxEvents  { fhicl::Name("maxEvents"), -1 };
      fhicl::Atom<int> maxSubRuns { fhicl::Name("maxSubRuns"), -1 };
      fhicl::Atom<int> reportFrequency { fhicl::Name("reportFrequency"), 1 };
      fhicl::Atom<bool> errorOnFailureToPut { fhicl::Name("errorOnFailureToPut"), false };
      fhicl::Atom<std::string> processingMode { fhicl::Name("processingMode"), defaultMode() };
    };

    DecrepitRelicInputSourceImplementation(fhicl::TableFragment<Config> const&,
                                           InputSourceDescription &);

    virtual ~DecrepitRelicInputSourceImplementation() noexcept = 0;

    // Note: this virtual function is implemented here. It is overridden
    // in RootInput, but may still be called under normal circumstances
    // by the overriding function.
    input::ItemType nextItemType() override;

    /// Read next event
    /// Indicate inability to get a new event by returning a null unique_ptr.
    std::unique_ptr<EventPrincipal> readEvent(cet::exempt_ptr<SubRunPrincipal const> srp) override;

    /// Read a specific event
    std::unique_ptr<EventPrincipal> readEvent(EventID const&) override;

    /// Read next subRun
    std::unique_ptr<SubRunPrincipal> readSubRun(cet::exempt_ptr<RunPrincipal const> rp) override;

    /// Read next run.
    std::unique_ptr<RunPrincipal> readRun() override;

    /// Read next file
    std::unique_ptr<FileBlock> readFile() override;

    /// close current file
    void closeFile() override;

    /// Skip the number of events specified.
    /// Offset may be negative.
    void skipEvents(int offset) override;

    /// Begin again at the first event
    void rewind() override {
      repeat_();
      doneReadAhead_ = false;
      state_ = input::IsInvalid;
      rewind_();
    }

    /// issue an event report
    void issueReports(EventID const& eventID);

    /// Reset the remaining number of events/subRuns to the maximum number.
    void repeat_() {
      remainingEvents_ = maxEvents_;
      remainingSubRuns_ = maxSubRuns_;
      doneReadAhead_ = false;
    }

    /// Accessor for maximum number of events to be read.
    /// -1 is used for unlimited.
    int maxEvents() const {return maxEvents_;}

    /// Accessor for remaining number of events to be read.
    /// -1 is used for unlimited.
    int remainingEvents() const {return remainingEvents_;}

    /// Accessor for maximum number of subRuns to be read.
    /// -1 is used for unlimited.
    int maxSubRuns() const {return maxSubRuns_;}

    /// Accessor for remaining number of subRuns to be read.
    /// -1 is used for unlimited.
    int remainingSubRuns() const {return remainingSubRuns_;}

    /// Called by framework at beginning of job
    void doBeginJob() override;

    /// Called by framework at end of job
    void doEndJob() override;

    /// Accessor for the current time, as seen by the input source
    Timestamp const& timestamp() const {return time_;}

    /// RunsSubRunsAndEvents (default), RunsAndSubRuns, or Runs.
    ProcessingMode processingMode() const {return processingMode_;}

  protected:
    /// To set the current time, as seen by the input source
    void setTimestamp(Timestamp const& theTime) {time_ = theTime;}

    input::ItemType state() const {return state_;}

    // Inform subclasses that they're going to be told to close the file
    // for non-exceptional reasons (such as hitting the event limit).
    virtual void finish() {}

    cet::exempt_ptr<RunPrincipal> runPrincipalExemptPtr() { return cachedRunPrincipal_; }
    cet::exempt_ptr<SubRunPrincipal> subRunPrincipalExemptPtr() { return cachedSubRunPrincipal_; }

    std::unique_ptr<RunPrincipal> runPrincipal()
    {
      cachedRunPrincipal_ = runPrincipal_.get();
      return std::move(runPrincipal_);
    }

    std::unique_ptr<SubRunPrincipal> subRunPrincipal()
    {
      cachedSubRunPrincipal_ = subRunPrincipal_.get();
      return std::move(subRunPrincipal_);
    }

    std::unique_ptr<EventPrincipal> eventPrincipal()
    {
      return std::move(eventPrincipal_);
    }

    void setRunPrincipal(std::unique_ptr<RunPrincipal>&& rp);
    void setSubRunPrincipal(std::unique_ptr<SubRunPrincipal>&& srp);
    void setEventPrincipal(std::unique_ptr<EventPrincipal>&& ep);
    void resetRunPrincipal() {runPrincipal_.reset();}
    void resetSubRunPrincipal() {subRunPrincipal_.reset();}
    void resetEventPrincipal() {eventPrincipal_.reset();}
    void reset() {
      doneReadAhead_ = false;
      state_ = input::IsInvalid;
    }

  private:
    bool eventLimitReached() const {return remainingEvents_ == 0;}
    bool subRunLimitReached() const {return remainingSubRuns_ == 0;}
    bool limitReached() const {return eventLimitReached() || subRunLimitReached();}
    virtual input::ItemType getNextItemType() = 0;
    input::ItemType nextItemType_();
    virtual std::unique_ptr<RunPrincipal> readRun_() = 0;
    virtual std::unique_ptr<SubRunPrincipal> readSubRun_() = 0;
    virtual std::unique_ptr<EventPrincipal> readEvent_() = 0;
    virtual std::unique_ptr<FileBlock> readFile_();
    virtual void closeFile_() {}
    virtual void skip(int);
    virtual void rewind_();
    virtual void beginJob();
    virtual void endJob();

  private:
    int maxEvents_;
    int maxSubRuns_;
    int const reportFrequency_;

    int remainingEvents_ {maxEvents_};
    int remainingSubRuns_ {maxSubRuns_};
    int readCount_ {};
    ProcessingMode processingMode_ {RunsSubRunsAndEvents};
    Timestamp time_ {Timestamp::invalidTimestamp()};
    bool doneReadAhead_ {false};
    input::ItemType state_ {input::IsInvalid};
    std::unique_ptr<RunPrincipal> runPrincipal_ {nullptr};
    std::unique_ptr<SubRunPrincipal> subRunPrincipal_ {nullptr};
    std::unique_ptr<EventPrincipal> eventPrincipal_ {nullptr};
    cet::exempt_ptr<RunPrincipal> cachedRunPrincipal_ {nullptr};
    cet::exempt_ptr<SubRunPrincipal> cachedSubRunPrincipal_ {nullptr};
  };  // DecrepitRelicInputSourceImplementation

}  // art

// ======================================================================

#endif /* art_Framework_Core_DecrepitRelicInputSourceImplementation_h */

// Local Variables:
// mode: c++
// End:
