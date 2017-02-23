// ======================================================================
//
// DecrepitRelicInputSourceImplementation
//
// ======================================================================

#include "art/Framework/Core/DecrepitRelicInputSourceImplementation.h"

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
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
  }  // namespace

  // ----------------------------------------------------------------------

  DecrepitRelicInputSourceImplementation::
  ~DecrepitRelicInputSourceImplementation()
  {}

  using DRISI = DecrepitRelicInputSourceImplementation;

  DecrepitRelicInputSourceImplementation::
  DecrepitRelicInputSourceImplementation(fhicl::TableFragment<DRISI::Config> const & config,
                                         InputSourceDescription & desc)
    : maxEvents_{config().maxEvents()}
    , maxSubRuns_{config().maxSubRuns()}
    , reportFrequency_{config().reportFrequency()}
    , moduleDescription_{desc.moduleDescription}
  {
    if (reportFrequency_ < 0) {
      throw art::Exception(art::errors::Configuration)
        << "reportFrequency has a negative value, which is not meaningful.";
    }
    std::string const runMode("Runs");
    std::string const runSubRunMode("RunsAndSubRuns");
    std::string const processingMode = config().processingMode();
    if (processingMode == runMode) {
      processingMode_ = Runs;
    }
    else if (processingMode == runSubRunMode) {
      processingMode_ = RunsAndSubRuns;
    }
    else if (processingMode != Config::defaultMode()) {
      throw art::Exception(art::errors::Configuration)
        << "DecrepitRelicInputSourceImplementation::DecrepitRelicInputSourceImplementation()\n"
        << "The 'processingMode' parameter for sources has an illegal value '"
        << processingMode << "'\n"
        << "Legal values are '" << Config::defaultMode()
        << "', '" << runSubRunMode
        << "', or '" << runMode << "'.\n";
    }

    // This must come LAST in the constructor.
    registerProducts(desc.productRegistry, moduleDescription_);
  }

  void
  DecrepitRelicInputSourceImplementation::setRunPrincipal(std::unique_ptr<RunPrincipal>&& rp)
  {
    runPrincipal_ = std::move(rp);
  }

  void
  DecrepitRelicInputSourceImplementation::setSubRunPrincipal(std::unique_ptr<SubRunPrincipal>&& srp)
  {
    assert(cachedRunPrincipal_);
    subRunPrincipal_ = std::move(srp);
    subRunPrincipal_->setRunPrincipal(cachedRunPrincipal_);
  }

  void
  DecrepitRelicInputSourceImplementation::setEventPrincipal(std::unique_ptr<EventPrincipal>&& ep)
  {
    assert(cachedSubRunPrincipal_);
    eventPrincipal_ = std::move(ep);
    eventPrincipal_->setSubRunPrincipal(cachedSubRunPrincipal_);
  }

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
  DecrepitRelicInputSourceImplementation::nextItemType_() {
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
  DecrepitRelicInputSourceImplementation::nextItemType() {
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
        runPrincipal_ = readRun_();
        state_ = input::IsRun;
      }
      else if (newState == input::IsSubRun || oldState == input::IsRun) {
        assert(processingMode() != Runs);
        subRunPrincipal_ = readSubRun_();
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
  DecrepitRelicInputSourceImplementation::doBeginJob() {
    beginJob();
  }

  void
  DecrepitRelicInputSourceImplementation::doEndJob() {
    endJob();
  }

  // Return a dummy file block.
  std::unique_ptr<FileBlock>
  DecrepitRelicInputSourceImplementation::readFile(MasterProductRegistry& /*mpr*/)
  {
    assert(doneReadAhead_);
    assert(state_ == input::IsFile);
    assert(!limitReached());
    doneReadAhead_ = false;
    return readFile_();
  }

  void
  DecrepitRelicInputSourceImplementation::closeFile() {
    return closeFile_();
  }

  // Return a dummy file block.
  // This function must be overridden for any input source that reads a file
  // containing Products. Such a function should update the MasterProductRegistry
  // to reflect the products found in this new file.
  std::unique_ptr<FileBlock>
  DecrepitRelicInputSourceImplementation::readFile_()
  {
    return std::make_unique<FileBlock>();
  }

  std::unique_ptr<RunPrincipal>
  DecrepitRelicInputSourceImplementation::readRun()
  {
    // Note: For the moment, we do not support saving and restoring the state of the
    // random number generator if random numbers are generated during processing of runs
    // (e.g. beginRun(), endRun())
    assert(doneReadAhead_);
    assert(state_ == input::IsRun);
    assert(!limitReached());
    doneReadAhead_ = false;
    cachedRunPrincipal_ = runPrincipal_.get();
    return std::move(runPrincipal_);
  }

  std::unique_ptr<SubRunPrincipal>
  DecrepitRelicInputSourceImplementation::readSubRun(cet::exempt_ptr<RunPrincipal const> rp)
  {
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
    cachedSubRunPrincipal_ = subRunPrincipal_.get();
    return std::move(subRunPrincipal_);
  }

  std::unique_ptr<EventPrincipal>
  DecrepitRelicInputSourceImplementation::readEvent(cet::exempt_ptr<SubRunPrincipal const> srp) {
    assert(doneReadAhead_);
    assert(state_ == input::IsEvent);
    assert(!eventLimitReached());
    doneReadAhead_ = false;

    eventPrincipal_ = readEvent_();
    assert(srp->run() == eventPrincipal_->run());
    assert(srp->subRun() == eventPrincipal_->subRun());
    eventPrincipal_->setSubRunPrincipal(srp);
    if (eventPrincipal_.get() != nullptr) {
      if (remainingEvents_ > 0) --remainingEvents_;
      ++readCount_;
      setTimestamp(eventPrincipal_->time());
      if ((reportFrequency_ > 0) &&
          ! (readCount_ % reportFrequency_))
      {
        issueReports(eventPrincipal_->id());
      }
    }
    return std::move(eventPrincipal_);
  }

  std::unique_ptr<EventPrincipal>
  DecrepitRelicInputSourceImplementation::readEvent(EventID const&) {
    throw art::Exception(art::errors::LogicError)
      << "DecrepitRelicInputSourceImplementation::readEvent()\n"
      << "Random access is not implemented for this type of Input Source\n"
      << "Contact a Framework Developer\n";
  }

  void
  DecrepitRelicInputSourceImplementation::skipEvents(int offset) {
    this->skip(offset);
  }

  void
  DecrepitRelicInputSourceImplementation::issueReports(EventID const& eventID) {
    time_t t = time(0);
    char ts[] = "dd-Mon-yyyy hh:mm:ss TZN     ";
    strftime( ts, strlen(ts)+1, "%d-%b-%Y %H:%M:%S %Z", localtime(&t) );
    mf::LogVerbatim("ArtReport")
      << "Begin processing the " << readCount_
      << suffix(readCount_) << " record. " << eventID
      << " at " << ts;
    // At some point we may want to initiate checkpointing here
  }

  void
  DecrepitRelicInputSourceImplementation::skip(int) {
    throw art::Exception(art::errors::LogicError)
      << "DecrepitRelicInputSourceImplementation::skip()\n"
      << "Random access is not implemented for this type of Input Source\n"
      << "Contact a Framework Developer\n";
  }

  void
  DecrepitRelicInputSourceImplementation::rewind_() {
    throw art::Exception(art::errors::LogicError)
      << "DecrepitRelicInputSourceImplementation::rewind()\n"
      << "Rewind is not implemented for this type of Input Source\n"
      << "Contact a Framework Developer\n";
  }

  void
  DecrepitRelicInputSourceImplementation::beginJob() { }

  void
  DecrepitRelicInputSourceImplementation::endJob() { }

}  // art

// ======================================================================
