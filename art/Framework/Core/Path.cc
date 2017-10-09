#include "art/Framework/Core/Path.h"

#include "art/Framework/Principal/Actions.h"
#include "cetlib/container_algorithms.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <algorithm>

using namespace cet;
using namespace fhicl;
using namespace std;

namespace art {

  Path::Path(int const bitpos,
             string const& path_name,
             WorkersInPath&& workers,
             TrigResPtr pathResults,
             ActionTable const& actions,
             ActivityRegistry& areg,
             bool const isEndPath)
    : bitpos_{bitpos}
    , name_{path_name}
    , trptr_{pathResults}
    , actReg_{areg}
    , act_table_{actions}
    , workers_{std::move(workers)}
    , isEndPath_{isEndPath}
  {}

  bool
  Path::handleWorkerFailure(cet::exception const& e,
                            int const nwrwue,
                            bool const isEvent)
  {
    bool should_continue{true};

    // there is no support as of yet for specific paths having
    // different exception behavior

    // If not processing an event, always rethrow.
    actions::ActionCodes action =
      (isEvent ? act_table_.find(e.root_cause()) : actions::Rethrow);
    assert(action != actions::FailModule);
    switch (action) {
      case actions::FailPath: {
        should_continue = false;
        mf::LogWarning(e.category())
          << "Failing path " << name_ << ", due to exception, message:\n"
          << e.what() << "\n";
        break;
      }
      default: {
        if (isEvent)
          ++timesExcept_;
        state_ = art::hlt::Exception;
        recordStatus(nwrwue, isEvent);
        throw art::Exception{
          errors::ScheduleExecutionFailure, "Path: ProcessingStopped.", e}
          << "Exception going through path " << name_ << "\n";
      }
    }

    return should_continue;
  }

  void
  Path::recordUnknownException(int const nwrwue, bool const isEvent)
  {
    mf::LogError("PassingThrough")
      << "Exception passing through path " << name_ << "\n";
    if (isEvent)
      ++timesExcept_;
    state_ = art::hlt::Exception;
    recordStatus(nwrwue, isEvent);
  }

  void
  Path::recordStatus(int const nwrwue, bool const isEvent)
  {
    if (isEvent && trptr_) {
      (*trptr_)[bitpos_] = HLTPathStatus(state_, nwrwue);
    }
  }

  void
  Path::updateCounters(bool const success, bool const isEvent)
  {
    if (success) {
      if (isEvent)
        ++timesPassed_;
      state_ = art::hlt::Pass;
    } else {
      if (isEvent)
        ++timesFailed_;
      state_ = art::hlt::Fail;
    }
  }

  void
  Path::clearCounters()
  {
    timesRun_ = timesPassed_ = timesFailed_ = timesExcept_ = 0;
    for_all(workers_, [](auto& w) { w.clearCounters(); });
  }

  void
  Path::findEventModifiers(std::vector<std::string>& foundLabels) const
  {
    findByModifiesEvent(true, foundLabels);
  }

  void
  Path::findEventObservers(std::vector<std::string>& foundLabels) const
  {
    findByModifiesEvent(false, foundLabels);
  }

  void
  Path::findByModifiesEvent(bool const modifies,
                            std::vector<std::string>& foundLabels) const
  {
    for (auto const& w : workers_) {
      if (w.modifiesEvent() == modifies) {
        foundLabels.push_back(w.label());
      }
    }
  }
}
