// ======================================================================
//
// InputSource
//
// ======================================================================

#include "art/Framework/Core/InputSource.h"

#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/Run.h"
#include "art/Framework/Core/RunPrincipal.h"
#include "art/Framework/Core/SubRun.h"
#include "art/Framework/Core/SubRunPrincipal.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#ifdef RNGS
//#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#endif
#include "art/Utilities/GlobalIdentifier.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "fhiclcpp/ParameterSet.h"
#include <cassert>
#include <ctime>

using fhicl::ParameterSet;

// ----------------------------------------------------------------------

namespace art {

  namespace {
    std::string const& suffix(int count) {
      static std::string const st("st");
      static std::string const nd("nd");
      static std::string const rd("rd");
      static std::string const th("th");
      // *0, *4 - *9 use "th".
      int lastDigit = count % 10;
      if (lastDigit >= 4 || lastDigit == 0) return th;
      // *11, *12, or *13 use "th".
      if (count % 100 - lastDigit == 10) return th;
      return (lastDigit == 1 ? st : (lastDigit == 2 ? nd : rd));
    }

    struct do_nothing_deleter {
      void  operator () (void const*) const { }
    };

    template <typename T>
    boost::shared_ptr<T> createSharedPtrToStatic(T * ptr) {
      return  boost::shared_ptr<T>(ptr, do_nothing_deleter());
    }
  }  // namespace

// ----------------------------------------------------------------------

  InputSource::InputSource( ParameterSet           const & pset
                          , InputSourceDescription const & desc )
  : ProductRegistryHelper( )
  , boost::noncopyable   ( )
  , actReg_              ( desc.actReg_ )
  , maxEvents_           ( desc.maxEvents_ )
  , remainingEvents_     ( maxEvents_ )
  , maxSubRuns_            ( desc.maxSubRuns_ )
  , remainingSubRuns_      ( maxSubRuns_ )
  , readCount_           ( 0 )
  , processingMode_      ( RunsSubRunsAndEvents )
  , moduleDescription_   ( desc.moduleDescription_ )
  , productRegistry_     ( createSharedPtrToStatic<ProductRegistry const>(desc.productRegistry_) )
  , primary_             ( pset.get<std::string>("module_label") == std::string("source") )
  , processGUID_         ( primary_ ? createGlobalIdentifier() : std::string() )
  , time_                ( )
  , doneReadAhead_       ( false )
  , state_               ( input::IsInvalid )
  , runPrincipal_        ( )
  , subRunPrincipal_       ( )
  {
    // Secondary input sources currently do not have a product registry.
    if (primary_) {
      assert(desc.productRegistry_ != 0);
    }
    std::string const defaultMode("RunsSubRunsAndEvents");
    std::string const runMode("Runs");
    std::string const runSubRunMode("RunsAndSubRuns");
    std::string processingMode
       = pset.get<std::string>("processingMode", defaultMode);
    if (processingMode == runMode) {
      processingMode_ = Runs;
    }
    else if (processingMode == runSubRunMode) {
      processingMode_ = RunsAndSubRuns;
    }
    else if (processingMode != defaultMode) {
      throw art::Exception(art::errors::Configuration)
        << "InputSource::InputSource()\n"
        << "The 'processingMode' parameter for sources has an illegal value '"
          << processingMode << "'\n"
        << "Legal values are '" << defaultMode
          << "', '" << runSubRunMode
          << "', or '" << runMode << "'.\n";
    }
  }


  void
  InputSource::commitEvent(art::Event &e)
  {
    e.commit_();
  }


  void
  InputSource::commitRun(art::Run &r)
  {
    r.commit_();
  }


  void
  InputSource::commitSubRun(art::SubRun &sr)
  {
    sr.commit_();
  }


  InputSource::~InputSource() { }

  // This next function is to guarantee that "runs only" mode does not
  // return events or subRuns, and that "runs and subRuns only" mode
  // does not return events.  For input sources that are not random
  // access (e.g. you need to read through the events to get to the
  // subRuns and runs), this is all that is involved to implement
  // these modes.  For input sources where events or subRuns can be
  // skipped, getNextItemType() should implement the skipping
  // internally, so that the performance gain is realized.  If this is
  // done for a source, the 'if' blocks in this function will never be
  // entered for that source.
  input::ItemType
  InputSource::nextItemType_() {
    input::ItemType itemType = getNextItemType();
    if (itemType == input::IsEvent && processingMode() != RunsSubRunsAndEvents) {
      readEvent_();
      return nextItemType_();
    }
    if (itemType == input::IsSubRun && processingMode() == Runs) {
      readSubRun_();
      return nextItemType_();
    }
    return itemType;
  }

  input::ItemType
  InputSource::nextItemType() {
    if (doneReadAhead_) {
      return state_;
    }
    doneReadAhead_ = true;
    input::ItemType oldState = state_;
    if (eventLimitReached()) {
      // If the maximum event limit has been reached, stop.
      state_ = input::IsStop;
    }
    else if (subRunLimitReached()) {
      // If the maximum subRun limit has been reached, stop
      // when reaching a new file, run, or subRun.
      if (oldState == input::IsInvalid ||
	  oldState == input::IsFile ||
	  oldState == input::IsRun ||
	  processingMode() != RunsSubRunsAndEvents) {
        state_ = input::IsStop;
      }
      else {
	input::ItemType newState = nextItemType_();
        if (newState == input::IsEvent) {
          assert(processingMode() == RunsSubRunsAndEvents);
          state_ = input::IsEvent;
        }
        else {
          state_ = input::IsStop;
        }
      }
    }
    else {
      input::ItemType newState = nextItemType_();
      if (newState == input::IsStop) {
        state_ = input::IsStop;
      }
      else if (newState == input::IsFile || oldState == input::IsInvalid) {
        state_ = input::IsFile;
      }
      else if (newState == input::IsRun || oldState == input::IsFile) {
        RunSourceSentry(*this);
        setRunPrincipal(readRun_());
        state_ = input::IsRun;
      }
      else if (newState == input::IsSubRun || oldState == input::IsRun) {
        assert(processingMode() != Runs);
        SubRunSourceSentry(*this);
        setSubRunPrincipal(readSubRun_());
        state_ = input::IsSubRun;
      }
      else {
        assert(processingMode() == RunsSubRunsAndEvents);
        state_ = input::IsEvent;
      }
    }
    if (state_ == input::IsStop) {
      subRunPrincipal_.reset();
      runPrincipal_.reset();
    }
    return state_;
  }

  void
  InputSource::doBeginJob() {
    beginJob();
  }

  void
  InputSource::doEndJob() {
    endJob();
  }

  void
  InputSource::registerProducts() {
    if (!typeLabelList().empty()) {
      addToRegistry(typeLabelList().begin(), typeLabelList().end(), moduleDescription(), productRegistryUpdate());
    }
  }

  // Return a dummy file block.
  boost::shared_ptr<FileBlock>
  InputSource::readFile() {
    assert(doneReadAhead_);
    assert(state_ == input::IsFile);
    assert(!limitReached());
    doneReadAhead_ = false;
    boost::shared_ptr<FileBlock> fb = readFile_();
    return fb;
  }

  void
  InputSource::closeFile() {
    return closeFile_();
  }

  // Return a dummy file block.
  // This function must be overridden for any input source that reads a file
  // containing Products.
  boost::shared_ptr<FileBlock>
  InputSource::readFile_() {
    return boost::shared_ptr<FileBlock>(new FileBlock);
  }

  boost::shared_ptr<RunPrincipal>
  InputSource::readRun() {
    // Note: For the moment, we do not support saving and restoring the state of the
    // random number generator if random numbers are generated during processing of runs
    // (e.g. beginRun(), endRun())
    assert(doneReadAhead_);
    assert(state_ == input::IsRun);
    assert(!limitReached());
    doneReadAhead_ = false;
    return runPrincipal_;
  }

  boost::shared_ptr<SubRunPrincipal>
  InputSource::readSubRun(boost::shared_ptr<RunPrincipal> rp) {
    // Note: For the moment, we do not support saving and restoring the state of the
    // random number generator if random numbers are generated during processing of subRuns
    // (e.g. beginSubRun(), endSubRun())
    assert(doneReadAhead_);
    assert(state_ == input::IsSubRun);
    assert(!limitReached());
    doneReadAhead_ = false;
    --remainingSubRuns_;
    assert(subRunPrincipal_->run() == rp->run());
    subRunPrincipal_->setRunPrincipal(rp);
    return subRunPrincipal_;
  }

  std::auto_ptr<EventPrincipal>
  InputSource::readEvent(boost::shared_ptr<SubRunPrincipal> srp) {
    assert(doneReadAhead_);
    assert(state_ == input::IsEvent);
    assert(!eventLimitReached());
    doneReadAhead_ = false;

    preRead();
    std::auto_ptr<EventPrincipal> result = readEvent_();
    assert(srp->run() == result->run());
    assert(srp->subRun() == result->subRun());
    result->setSubRunPrincipal(srp);
    if (result.get() != 0) {
      Event event(*result, moduleDescription());
      postRead(event);
      if (remainingEvents_ > 0) --remainingEvents_;
      ++readCount_;
      setTimestamp(result->time());
      issueReports(result->id(), result->subRun());
    }
    return result;
  }

  std::auto_ptr<EventPrincipal>
  InputSource::readEvent(EventID const& eventID) {
    std::auto_ptr<EventPrincipal> result(0);

    if (!limitReached()) {
      preRead();
      result = readIt(eventID);
      if (result.get() != 0) {
        Event event(*result, moduleDescription());
        postRead(event);
        if (remainingEvents_ > 0) --remainingEvents_;
        ++readCount_;
        issueReports(result->id(), result->subRun());
      }
    }
    return result;
  }

  void
  InputSource::skipEvents(int offset) {
    this->skip(offset);
  }

  void
  InputSource::issueReports(EventID const& eventID, SubRunNumber_t const& subRun) {
    time_t t = time(0);
    char ts[] = "dd-Mon-yyyy hh:mm:ss TZN     ";
    strftime( ts, strlen(ts)+1, "%d-%b-%Y %H:%M:%S %Z", localtime(&t) );
    mf::LogVerbatim("ArtReport")
      << "Begin processing the " << readCount_
      << suffix(readCount_) << " record. Run " << eventID.run()
      << ", Event " << eventID.event()
      << ", SubRun " << subRun<< " at " << ts;
    // At some point we may want to initiate checkpointing here
  }

  std::auto_ptr<EventPrincipal>
  InputSource::readIt(EventID const&) {
    throw art::Exception(art::errors::LogicError)
      << "InputSource::readIt()\n"
      << "Random access is not implemented for this type of Input Source\n"
      << "Contact a Framework Developer\n";
  }

  void
  InputSource::skip(int) {
    throw art::Exception(art::errors::LogicError)
      << "InputSource::skip()\n"
      << "Random access is not implemented for this type of Input Source\n"
      << "Contact a Framework Developer\n";
  }

  void
  InputSource::rewind_() {
    throw art::Exception(art::errors::LogicError)
      << "InputSource::rewind()\n"
      << "Rewind is not implemented for this type of Input Source\n"
      << "Contact a Framework Developer\n";
  }

  void
  InputSource::preRead() {  // roughly corresponds to "end of the prev event"
#ifdef RNGS
    if (primary()) {
      ServiceHandle<RandomNumberGenerator> rng;
      if (rng.isAvailable()) {
        rng->takeSnapshot_();
      }
    }
#endif
  }

  void
  InputSource::postRead(Event& event) {
#ifdef RNGS
    if (primary()) {
      ServiceHandle<RandomNumberGenerator> rng;
      if (rng.isAvailable()) {
        rng->restoreSnapshot_(event);
      }
    }
#endif
  }

  void
  InputSource::doEndRun(RunPrincipal& rp) {
    rp.setEndTime(time_);
    Run run(rp, moduleDescription());
    endRun(run);
    run.commit_();
  }

  void
  InputSource::doEndSubRun(SubRunPrincipal & srp) {
    srp.setEndTime(time_);
    SubRun sr(srp, moduleDescription());
    endSubRun(sr);
    sr.commit_();
  }

//   void
//   InputSource::wakeUp_() { }

  void
  InputSource::endSubRun(SubRun &) { }

  void
  InputSource::endRun(Run &) { }

  void
  InputSource::beginJob() { }

  void
  InputSource::endJob() { }

  RunNumber_t
  InputSource::run() const {
    assert(runPrincipal());
    return runPrincipal()->run();
  }

  SubRunNumber_t
  InputSource::subRun() const {
    assert(subRunPrincipal());
    return subRunPrincipal()->subRun();
  }


  InputSource::SourceSentry::SourceSentry(Sig& pre, Sig& post) : post_(post) {
    pre();
  }

  InputSource::SourceSentry::~SourceSentry() {
    post_();
  }

  InputSource::EventSourceSentry::EventSourceSentry(InputSource const& source) :
     sentry_(source.actReg()->preSourceSignal_, source.actReg()->postSourceSignal_) {
  }

  InputSource::SubRunSourceSentry::SubRunSourceSentry(InputSource const& source) :
     sentry_(source.actReg()->preSourceSubRunSignal_, source.actReg()->postSourceSubRunSignal_) {
  }

  InputSource::RunSourceSentry::RunSourceSentry(InputSource const& source) :
     sentry_(source.actReg()->preSourceRunSignal_, source.actReg()->postSourceRunSignal_) {
  }

  InputSource::FileOpenSentry::FileOpenSentry(InputSource const& source) :
     sentry_(source.actReg()->preOpenFileSignal_, source.actReg()->postOpenFileSignal_) {
  }

  InputSource::FileCloseSentry::FileCloseSentry(InputSource const& source) :
     sentry_(source.actReg()->preCloseFileSignal_, source.actReg()->postCloseFileSignal_) {
  }

}  // art

// ======================================================================
