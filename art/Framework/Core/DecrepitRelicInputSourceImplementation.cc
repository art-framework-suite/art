#include "art/Framework/Core/DecrepitRelicInputSourceImplementation.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <cassert>
#include <ctime>
#include <memory>
#include <string>

using namespace std;
using fhicl::ParameterSet;

namespace art {

  namespace {

    string const&
    suffix(int count)
    {
      static string const st("st");
      static string const nd("nd");
      static string const rd("rd");
      static string const th("th");
      // *0, *4 - *9 use "th".
      int lastDigit = count % 10;
      if (lastDigit >= 4 || lastDigit == 0) {
        return th;
      }
      // *11, *12, or *13 use "th".
      if (count % 100 - lastDigit == 10) {
        return th;
      }
      return ((lastDigit == 1) ? st : ((lastDigit == 2) ? nd : rd));
    }

  } // unnamed namespace

  DecrepitRelicInputSourceImplementation::Config::
  ~Config()
  {
  }

  DecrepitRelicInputSourceImplementation::Config::
  Config()
    : maxEvents{fhicl::Name("maxEvents"), -1}
    , maxSubRuns{fhicl::Name("maxSubRuns"), -1}
    , reportFrequency{fhicl::Name("reportFrequency"), 1}
    , errorOnFailureToPut{fhicl::Name("errorOnFailureToPut"), false }
    , processingMode{fhicl::Name("processingMode"), defaultMode()}
  {
  }

  // Note: static.
  char const*
  DecrepitRelicInputSourceImplementation::Config::
  defaultMode()
  {
    return "RunsSubRunsAndEvents";
  }

  DecrepitRelicInputSourceImplementation::
  ~DecrepitRelicInputSourceImplementation() noexcept
  {
  }

  DecrepitRelicInputSourceImplementation::
  DecrepitRelicInputSourceImplementation(fhicl::TableFragment<DecrepitRelicInputSourceImplementation::Config> const& config,
                                         ModuleDescription const& desc)
    : InputSource{desc}
    , maxEvents_{config().maxEvents()}
    , maxSubRuns_{config().maxSubRuns()}
    , reportFrequency_{config().reportFrequency()}
    , remainingEvents_{maxEvents_}
    , remainingSubRuns_{maxSubRuns_}
  {
    if (reportFrequency_ < 0) {
      throw art::Exception(art::errors::Configuration)
        << "reportFrequency has a negative value, which is not meaningful.";
    }
    std::string const runMode{"Runs"};
    std::string const runSubRunMode{"RunsAndSubRuns"};
    std::string const processingMode = config().processingMode();
    if (processingMode == runMode) {
      processingMode_ = Runs;
    }
    else if (processingMode == "RunsAndSubRuns") {
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
  }

  input::ItemType
  DecrepitRelicInputSourceImplementation::
  nextItemType()
  {
    auto const oldState = state_;
    if (remainingEvents_ == 0) {
      // If the maximum event limit has been reached, stop.
      state_ = input::IsStop;
      return state_;
    }
    if (remainingSubRuns_ == 0) {
      // If the maximum subRun limit has been reached, stop
      // when reaching a new file, run, or subRun.
      if (oldState == input::IsInvalid ||
          oldState == input::IsFile ||
          oldState == input::IsRun ||
          processingMode_ != RunsSubRunsAndEvents) {
        state_ = input::IsStop;
        return state_;
      }
    }
    auto newState = getNextItemType();
    while (1) {
      if ((newState == input::IsEvent) && (processingMode_ != RunsSubRunsAndEvents)) {
        newState = getNextItemType();
        continue;
      }
      if ((newState == input::IsSubRun) && (processingMode_ == Runs)) {
        newState = getNextItemType();
        continue;
      }
      break;
    }
    if (newState == input::IsStop) {
      state_ = input::IsStop;
      // FIXME: upon the advent of a catalog system which can do
      // something intelligent with the difference between whole-file
      // success, partial-file success, partial-file failure and
      // whole-file failure (such as file-open failure), we will need to
      // communicate that difference here. The file disposition options
      // as they are now (and the mapping to any concrete implementation
      // we are are aware of currently) are not sufficient to the task,
      // so we deliberately do not distinguish here between partial-file
      // and whole-file success in particular.
      finish();
      return state_;
    }
    else if (newState == input::IsFile || oldState == input::IsInvalid) {
      state_ = input::IsFile;
      return state_;
    }
    else if (newState == input::IsRun || oldState == input::IsFile) {
      state_ = input::IsRun;
      return state_;
    }
    else if (newState == input::IsSubRun || oldState == input::IsRun) {
      assert(processingMode_ != Runs);
      state_ = input::IsSubRun;
      return state_;
    }
    assert(processingMode_ == RunsSubRunsAndEvents);
    state_ = input::IsEvent;
    return state_;
  }

  // Return a dummy file block.
  std::unique_ptr<FileBlock>
  DecrepitRelicInputSourceImplementation::readFile()
  {
    assert(state_ == input::IsFile);
    assert((remainingEvents_ != 0) && (remainingSubRuns_ != 0));
    return readFile_();
  }

  // Return a dummy file block.
  // This function must be overridden for any input source that reads a file
  // containing Products. Such a function should update the UpdateOutputCallbacks
  // to reflect the products found in this new file.
  unique_ptr<FileBlock>
  DecrepitRelicInputSourceImplementation::
  readFile_()
  {
    return make_unique<FileBlock>();
  }

  void
  DecrepitRelicInputSourceImplementation::
  closeFile()
  {
    return closeFile_();
  }

  void
  DecrepitRelicInputSourceImplementation::
  closeFile_()
  {
  }

  unique_ptr<RunPrincipal>
  DecrepitRelicInputSourceImplementation::
  readRun()
  {
    // Note: For the moment, we do not support saving and restoring the state of the
    // random number generator if random numbers are generated during processing of runs
    // (e.g. beginRun(), endRun())
    assert(state_ == input::IsRun);
    assert((remainingEvents_ != 0) && (remainingSubRuns_ != 0));
    return readRun_();
  }

  unique_ptr<SubRunPrincipal>
  DecrepitRelicInputSourceImplementation::
  readSubRun(cet::exempt_ptr<RunPrincipal const> rp)
  {
    // Note: For the moment, we do not support saving and restoring the state of the
    // random number generator if random numbers are generated during processing of subRuns
    // (e.g. beginSubRun(), endSubRun())
    assert(state_ == input::IsSubRun);
    assert((remainingEvents_ != 0) && (remainingSubRuns_ != 0));
    --remainingSubRuns_;
    auto srp = readSubRun_(rp);
    srp->setRunPrincipal(rp);
    return srp;
  }

  unique_ptr<EventPrincipal>
  DecrepitRelicInputSourceImplementation::
  readEvent(cet::exempt_ptr<SubRunPrincipal const> srp)
  {
    assert(state_ == input::IsEvent);
    assert(remainingEvents_ != 0);
    auto ep = readEvent_();
    assert(srp->run() == ep->run());
    assert(srp->subRun() == ep->subRun());
    ep->setSubRunPrincipal(srp);
    if (ep.get() != nullptr) {
      if (remainingEvents_ > 0) {
        --remainingEvents_;
      }
      ++numberOfEventsRead_;
      if ((reportFrequency_ > 0) && !(numberOfEventsRead_ % reportFrequency_)) {
        issueReports(ep->eventID());
      }
    }
    return ep;
  }

  void
  DecrepitRelicInputSourceImplementation::
  doBeginJob()
  {
    beginJob();
  }

  void
  DecrepitRelicInputSourceImplementation::
  beginJob()
  {
  }

  void
  DecrepitRelicInputSourceImplementation::
  doEndJob()
  {
    endJob();
  }

  void
  DecrepitRelicInputSourceImplementation::
  endJob()
  {
  }

  unique_ptr<EventPrincipal>
  DecrepitRelicInputSourceImplementation::
  readEvent(EventID const&)
  {
    throw art::Exception(art::errors::LogicError)
      << "DecrepitRelicInputSourceImplementation::readEvent()\n"
      << "Random access is not implemented for this type of Input Source\n"
      << "Contact a Framework Developer\n";
  }

  void
  DecrepitRelicInputSourceImplementation::
  skipEvents(int offset)
  {
    skip(offset);
  }

  void
  DecrepitRelicInputSourceImplementation::
  skip(int)
  {
    throw art::Exception(art::errors::LogicError)
      << "DecrepitRelicInputSourceImplementation::skip()\n"
      << "Random access is not implemented for this type of Input Source\n"
      << "Contact a Framework Developer\n";
  }

  // Begin again at the first event
  void
  DecrepitRelicInputSourceImplementation::
  rewind()
  {
    remainingEvents_ = maxEvents_;
    remainingSubRuns_ = maxSubRuns_;
    state_ = input::IsInvalid;
    rewind_();
  }

  void
  DecrepitRelicInputSourceImplementation::
  rewind_()
  {
    throw art::Exception(art::errors::LogicError)
      << "DecrepitRelicInputSourceImplementation::rewind()\n"
      << "Rewind is not implemented for this type of Input Source\n"
      << "Contact a Framework Developer\n";
  }

  // RunsSubRunsAndEvents (default), RunsAndSubRuns, or Runs.
  InputSource::ProcessingMode
  DecrepitRelicInputSourceImplementation::
  processingMode() const
  {
    return processingMode_;
  }

  // Accessor for maximum number of events to be read.
  // -1 is used for unlimited.
  int
  DecrepitRelicInputSourceImplementation::
  maxEvents() const
  {
    return maxEvents_;
  }

  // Accessor for remaining number of events to be read.
  // -1 is used for unlimited.
  int
  DecrepitRelicInputSourceImplementation::
  remainingEvents() const
  {
    return remainingEvents_;
  }

  // Accessor for maximum number of subRuns to be read.
  // -1 is used for unlimited.
  int
  DecrepitRelicInputSourceImplementation::
  maxSubRuns() const
  {
    return maxSubRuns_;
  }

  // Accessor for remaining number of subRuns to be read.
  // -1 is used for unlimited.
  int
  DecrepitRelicInputSourceImplementation::
  remainingSubRuns() const
  {
    return remainingSubRuns_;
  }

  // Reset the remaining number of events/subRuns to the maximum number.
  void
  DecrepitRelicInputSourceImplementation::
  repeat_()
  {
    remainingEvents_ = maxEvents_;
    remainingSubRuns_ = maxSubRuns_;
  }

  void
  DecrepitRelicInputSourceImplementation::
  issueReports(EventID const& eventID)
  {
    time_t t = time(0);
    char ts[] = "dd-Mon-yyyy hh:mm:ss TZN     ";
    strftime(ts, strlen(ts) + 1, "%d-%b-%Y %H:%M:%S %Z", localtime(&t));
    mf::LogVerbatim("ArtReport")
      << "Begin processing the " << numberOfEventsRead_
      << suffix(numberOfEventsRead_) << " record. " << eventID
      << " at " << ts;
    // At some point we may want to initiate checkpointing here
  }

  input::ItemType
  DecrepitRelicInputSourceImplementation::
  state() const
  {
    return state_;
  }

  void
  DecrepitRelicInputSourceImplementation::
  setState(input::ItemType state)
  {
    state_ = state;
  }

  void
  DecrepitRelicInputSourceImplementation::
  reset()
  {
    state_ = input::IsInvalid;
  }

} // namespace art
