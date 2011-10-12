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
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "art/Persistency/Provenance/Timestamp.h"
#include "boost/noncopyable.hpp"
#include "cetlib/exempt_ptr.h"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"
#include "sigc++/signal.h"
#include <string>

// ----------------------------------------------------------------------

namespace art {
  class ActivityRegistry;
  class MasterProductRegistry;

  class DecrepitRelicInputSourceImplementation :
    public InputSource,
    private ProductRegistryHelper,
    private boost::noncopyable {
  public:

    DecrepitRelicInputSourceImplementation(fhicl::ParameterSet const &,
                                           InputSourceDescription &);

    virtual ~DecrepitRelicInputSourceImplementation() = 0;

    // Note: this virtual function is implemented here. It is overridden
    // in RootInput, but may still be called under normal circumstances
    // by the overriding function.
    virtual input::ItemType nextItemType();

    /// Read next event
    /// Indicate inability to get a new event by returning a null auto_ptr.
    std::auto_ptr<EventPrincipal> readEvent(std::shared_ptr<SubRunPrincipal> srp);

    /// Read a specific event
    std::auto_ptr<EventPrincipal> readEvent(EventID const &);

    /// Read next subRun
    std::shared_ptr<SubRunPrincipal> readSubRun(std::shared_ptr<RunPrincipal> rp);

    /// Read next run.
    std::shared_ptr<RunPrincipal> readRun();

    /// Read next file
    std::shared_ptr<FileBlock> readFile(MasterProductRegistry &);

    /// close current file
    void closeFile();

    /// Skip the number of events specified.
    /// Offset may be negative.
    void skipEvents(int offset);

    /// Begin again at the first event
    void rewind() {
      repeat_();
      doneReadAhead_ = false;
      state_ = input::IsInvalid;
      rewind_();
    }

    /// Wake up the input source
    //void wakeUp() {wakeUp_();}

    /// issue an event report
    void issueReports(EventID const & eventID);

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

    /// Accessor for 'module' description.
    ModuleDescription const & moduleDescription() const {return moduleDescription_;}

    /// Accessor for Process Configuration
    ProcessConfiguration const & processConfiguration() const {return moduleDescription().processConfiguration();}

    /// Called by framework at beginning of job
    void doBeginJob();

    /// Called by framework at end of job
    void doEndJob();

    /// Called by framework when events are exhausted.
    void doEndSubRun(SubRunPrincipal & srp);
    void doEndRun(RunPrincipal & rp);

    /// Accessor for the current time, as seen by the input source
    Timestamp const & timestamp() const {return time_;}

    /// Accessor for current run number
    RunNumber_t run() const;

    /// Accessor for current subRun number
    SubRunNumber_t subRun() const;

    /// RunsSubRunsAndEvents (default), RunsAndSubRuns, or Runs.
    ProcessingMode processingMode() const {return processingMode_;}

    /// Accessor for Activity Registry
    //std::shared_ptr<ActivityRegistry> actReg() const {return actReg_;}

    using ProductRegistryHelper::reconstitutes;

  protected:
    /// To set the current time, as seen by the input source
    void setTimestamp(Timestamp const & theTime) {time_ = theTime;}

    input::ItemType state() const {return state_;}
    std::shared_ptr<RunPrincipal> runPrincipal() const {return runPrincipal_;}
    std::shared_ptr<SubRunPrincipal> subRunPrincipal() const {return subRunPrincipal_;}
    void setRunPrincipal(std::shared_ptr<RunPrincipal> rp) {runPrincipal_ = rp;}
    void setSubRunPrincipal(std::shared_ptr<SubRunPrincipal> srp) {subRunPrincipal_ = srp;}
    void resetRunPrincipal() {runPrincipal_.reset();}
    void resetSubRunPrincipal() {subRunPrincipal_.reset();}
    void reset() {
      doneReadAhead_ = false;
      state_ = input::IsInvalid;
    }

    // To call the private commit_() functions of classes with which we are friends
    void commitEvent(Event & e);
    void commitRun(Run & r);
    void commitSubRun(SubRun & sr);

  private:
    bool eventLimitReached() const {return remainingEvents_ == 0;}
    bool subRunLimitReached() const {return remainingSubRuns_ == 0;}
    bool limitReached() const {return eventLimitReached() || subRunLimitReached();}
    virtual input::ItemType getNextItemType() = 0;
    input::ItemType nextItemType_();
    virtual std::shared_ptr<RunPrincipal> readRun_() = 0;
    virtual std::shared_ptr<SubRunPrincipal> readSubRun_() = 0;
    virtual std::auto_ptr<EventPrincipal> readEvent_() = 0;
    // Only subclasses that actually read a file will need to use the
    // MasterProductRegistry.
    virtual std::shared_ptr<FileBlock> readFile_(MasterProductRegistry &);
    virtual void closeFile_() {}
    virtual void skip(int);
    virtual void rewind_();
    //virtual void wakeUp_();
    void preRead();
    void postRead(Event & event);
    virtual void endSubRun(SubRun &);
    virtual void endRun(Run &);
    virtual void beginJob();
    virtual void endJob();

  private:
    std::shared_ptr<ActivityRegistry> actReg_;
    int maxEvents_;
    int remainingEvents_;
    int maxSubRuns_;
    int remainingSubRuns_;
    int readCount_;
    ProcessingMode processingMode_;
    ModuleDescription const moduleDescription_;
    Timestamp time_;
    bool doneReadAhead_;
    input::ItemType state_;
    std::shared_ptr<RunPrincipal>  runPrincipal_;
    std::shared_ptr<SubRunPrincipal>  subRunPrincipal_;
    bool usingSequentialAccess_;
  };  // DecrepitRelicInputSourceImplementation

}  // art

// ======================================================================

#endif /* art_Framework_Core_DecrepitRelicInputSourceImplementation_h */

// Local Variables:
// mode: c++
// End:
