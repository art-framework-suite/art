#ifndef FWCore_Framework_InputSource_h
#define FWCore_Framework_InputSource_h

/*----------------------------------------------------------------------

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

 1) EmptySource: creates EventPrincipals which contain no EDProducts.
 2) PoolSource: creates EventPrincipals which "contain" the data
    read from a POOL file. This source should provide for delayed loading
    of data, thus the quotation marks around contain.
 3) DAQInputSource: creats EventPrincipals which contain raw data, as
    delivered by the L1 trigger and event builder.

----------------------------------------------------------------------*/

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "art/Persistency/Provenance/Timestamp.h"
#include "boost/noncopyable.hpp"
#include "boost/shared_ptr.hpp"
#include "fhiclcpp/ParameterSet.h"
#include "sigc++/signal.h"
#include <memory>
#include <string>

// ----------------------------------------------------------------------

namespace art {
  class ActivityRegistry;

  class InputSource
    : private ProductRegistryHelper
    , private boost::noncopyable
  {
  public:
    enum ItemType {
        IsInvalid,
        IsStop,
        IsFile,
        IsRun,
        IsSubRun,
        IsEvent,
        IsRepeat
    };

    enum ProcessingMode {
        Runs,
        RunsAndSubRuns,
        RunsSubRunsAndEvents
    };

    typedef ProductRegistryHelper::TypeLabelList TypeLabelList;
    /// Constructor
    explicit InputSource(fhicl::ParameterSet const&,
                         InputSourceDescription const&);

    /// Destructor
    virtual ~InputSource();

    ItemType nextItemType();

    /// Read next event
    /// Indicate inability to get a new event by returning a null auto_ptr.
    std::auto_ptr<EventPrincipal> readEvent(boost::shared_ptr<SubRunPrincipal> lbp);

    /// Read a specific event
    std::auto_ptr<EventPrincipal> readEvent(EventID const&);

    /// Read next subRun
    boost::shared_ptr<SubRunPrincipal> readSubRun(boost::shared_ptr<RunPrincipal> rp);

    /// Read next run
    boost::shared_ptr<RunPrincipal> readRun();

    /// Read next file
    boost::shared_ptr<FileBlock> readFile();

    /// close current file
    void closeFile();

    /// Skip the number of events specified.
    /// Offset may be negative.
    void skipEvents(int offset);

    /// Begin again at the first event
    void rewind() {
      doneReadAhead_ = false;
      state_ = IsInvalid;
      rewind_();
    }

    /// Wake up the input source
    void wakeUp() {wakeUp_();}

    /// Set the run number
    void setRunNumber(RunNumber_t r) {setRun(r);}

    /// Set the subRun ID
    void setSubRunNumber_t(SubRunNumber_t lb) {setSubRun(lb);}

    /// issue an event report
    void issueReports(EventID const& eventID, SubRunNumber_t const& subRun);

    /// Register any produced products
    void registerProducts();

    /// Accessor for product registry.
    boost::shared_ptr<ProductRegistry const> productRegistry() const {return productRegistry_;}

    /// Reset the remaining number of events/subRuns to the maximum number.
    void repeat() {
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
    ModuleDescription const& moduleDescription() const {return moduleDescription_;}

    /// Accessor for Process Configuration
    ProcessConfiguration const& processConfiguration() const {return moduleDescription().processConfiguration();}

    /// Accessor for primary input source flag
    bool const primary() const {return primary_;}

    /// Accessor for global process identifier
    std::string const& processGUID() const {return processGUID_;}

    /// Called by framework at beginning of job
    void doBeginJob();

    /// Called by framework at end of job
    void doEndJob();

    /// Called by framework when events are exhausted.
    void doEndSubRun(SubRunPrincipal& lbp);
    void doEndRun(RunPrincipal& rp);

    /// Accessor for the current time, as seen by the input source
    Timestamp const& timestamp() const {return time_;}

    /// Accessor for current run number
    RunNumber_t run() const;

    /// Accessor for current subRun number
    SubRunNumber_t subRun() const;

    /// RunsSubRunsAndEvents (default), RunsAndSubRuns, or Runs.
    ProcessingMode processingMode() const {return processingMode_;}

    /// Accessor for Activity Registry
    boost::shared_ptr<ActivityRegistry> actReg() const {return actReg_;}

    using ProductRegistryHelper::produces;
    using ProductRegistryHelper::typeLabelList;

    class SourceSentry : private boost::noncopyable {
    public:
      typedef sigc::signal<void> Sig;
      SourceSentry(Sig& pre, Sig& post);
      ~SourceSentry();
    private:
      Sig& post_;
    };

    class EventSourceSentry {
    public:
      explicit EventSourceSentry(InputSource const& source);
    private:
      SourceSentry sentry_;
    };

    class SubRunSourceSentry {
    public:
      explicit SubRunSourceSentry(InputSource const& source);
    private:
      SourceSentry sentry_;
    };

    class RunSourceSentry {
    public:
      explicit RunSourceSentry(InputSource const& source);
    private:
      SourceSentry sentry_;
    };

    class FileOpenSentry {
    public:
      explicit FileOpenSentry(InputSource const& source);
    private:
      SourceSentry sentry_;
    };

    class FileCloseSentry {
    public:
      explicit FileCloseSentry(InputSource const& source);
    private:
      SourceSentry sentry_;
    };

  protected:
    /// To set the current time, as seen by the input source
    void setTimestamp(Timestamp const& theTime) {time_ = theTime;}

    ProductRegistry & productRegistryUpdate() const {return const_cast<ProductRegistry &>(*productRegistry_);}
    ItemType state() const{return state_;}
    boost::shared_ptr<RunPrincipal> runPrincipal() const {return runPrincipal_;}
    boost::shared_ptr<SubRunPrincipal> subRunPrincipal() const {return subRunPrincipal_;}
    void setRunPrincipal(boost::shared_ptr<RunPrincipal> rp) {runPrincipal_ = rp;}
    void setSubRunPrincipal(boost::shared_ptr<SubRunPrincipal> lbp) {subRunPrincipal_ = lbp;}
    void resetRunPrincipal() {runPrincipal_.reset();}
    void resetSubRunPrincipal() {subRunPrincipal_.reset();}
    void reset() const {
      doneReadAhead_ = false;
      state_ = IsInvalid;
    }

    // To call the private commit_() functions of classes with which we are friends
    void commitEvent(Event &e);
    void commitRun(Run &r);
    void commitSubRun(SubRun &lb);

  private:
    bool eventLimitReached() const {return remainingEvents_ == 0;}
    bool subRunLimitReached() const {return remainingSubRuns_ == 0;}
    bool limitReached() const {return eventLimitReached() || subRunLimitReached();}
    virtual ItemType getNextItemType() = 0;
    ItemType nextItemType_();
    virtual boost::shared_ptr<RunPrincipal> readRun_() = 0;
    virtual boost::shared_ptr<SubRunPrincipal> readSubRun_() = 0;
    virtual std::auto_ptr<EventPrincipal> readEvent_() = 0;
    virtual std::auto_ptr<EventPrincipal> readIt(EventID const&);
    virtual boost::shared_ptr<FileBlock> readFile_();
    virtual void closeFile_() {}
    virtual void skip(int);
    virtual void setRun(RunNumber_t r);
    virtual void setSubRun(SubRunNumber_t lb);
    virtual void rewind_();
    virtual void wakeUp_();
    void preRead();
    void postRead(Event& event);
    virtual void endSubRun(SubRun &);
    virtual void endRun(Run &);
    virtual void beginJob();
    virtual void endJob();

  private:
    boost::shared_ptr<ActivityRegistry> actReg_;
    int maxEvents_;
    int remainingEvents_;
    int maxSubRuns_;
    int remainingSubRuns_;
    int readCount_;
    ProcessingMode processingMode_;
    ModuleDescription const moduleDescription_;
    boost::shared_ptr<ProductRegistry const> productRegistry_;
    bool const primary_;
    std::string processGUID_;
    Timestamp time_;
    mutable bool doneReadAhead_;
    mutable ItemType state_;
    mutable boost::shared_ptr<RunPrincipal>  runPrincipal_;
    mutable boost::shared_ptr<SubRunPrincipal>  subRunPrincipal_;
  };  // InputSource

}  // art

// ======================================================================

#endif
