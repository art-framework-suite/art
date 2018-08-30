#include "art/Framework/Core/ProcessAndEventSelectors.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/EventSelector.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Common/TriggerResults.h"
#include "cetlib_except/exception.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace cet;
using namespace std;
using namespace string_literals;

namespace art::detail {

  ProcessAndEventSelector::~ProcessAndEventSelector() = default;

  ProcessAndEventSelector::ProcessAndEventSelector(string const& nm,
                                                   EventSelector const& es)
    : processNameSelector_{nm}, eventSelector_{es}
  {}

  void
  ProcessAndEventSelector::loadTriggerResults(Event const& e)
  {
    e.get(processNameSelector_, triggerResults_);
  }

  Handle<TriggerResults>
  ProcessAndEventSelector::triggerResults() const
  {
    return triggerResults_;
  }

  void
  ProcessAndEventSelector::clearTriggerResults()
  {
    triggerResults_.clear();
  }

  bool
  ProcessAndEventSelector::match()
  {
    return eventSelector_.acceptEvent(*triggerResults_);
  }

  ProcessAndEventSelectors::~ProcessAndEventSelectors() = default;
  ProcessAndEventSelectors::ProcessAndEventSelectors() = default;

  void
  ProcessAndEventSelectors::setupDefault(
    vector<string> const& trigger_path_names)
  {
    // Setup to accept everything.
    vector<string> paths;
    EventSelector es{paths, trigger_path_names};
    sel_.emplace_back(""s, std::move(es));
  }

  void
  ProcessAndEventSelectors::setup(
    vector<pair<string, string>> const& path_specs,
    vector<string> const& triggernames,
    string const& process_name)
  {
    // Turn the passed path specs into a map of process name to
    // a vector of trigger names.
    map<string, vector<string>> paths_for_process;
    for (auto const& pr : path_specs) {
      auto const& pname = pr.first.empty() ? process_name : pr.first;
      paths_for_process[pname].push_back(pr.second);
    }
    // Now go through all the process names found, and create an event
    // selector for each one.
    for (auto const & [pname, paths] : paths_for_process) {
      if (pname == process_name) {
        // For the passed process name we have been given the trigger names.
        sel_.emplace_back(pname, EventSelector{paths, triggernames});
        continue;
      }
      // For other process names we do not know the trigger names.
      sel_.emplace_back(pname, EventSelector{paths});
    }
  }

  bool
  ProcessAndEventSelectors::wantEvent(Event const& ev)
  {
    // Get all the TriggerResults objects before we test any for a
    // match, because we have to deal with the possibility there may be
    // more than one.  Note that the existence of more than one object
    // in the event is intended to lead to an exception throw *unless*
    // either the configuration has been set to match all events, or the
    // configuration is set to use specific process names.
    if (!loadDone_) {
      loadDone_ = true;
      // Note: The loadTriggerResults call might throw,
      // so numberFound_ may be less than expected.
      for (auto& val : sel_) {
        val.loadTriggerResults(ev);
        ++numberFound_;
      }
    }
    for (auto& val : sel_) {
      if (val.match()) {
        return true;
      }
    }
    return false;
  }

  Handle<TriggerResults>
  ProcessAndEventSelectors::getOneTriggerResults(Event const& ev)
  {
    auto This = const_cast<ProcessAndEventSelectors*>(this);
    if (!loadDone_) {
      loadDone_ = true;
      // Note: The loadTriggerResults call might throw,
      // so numberFound_ may be less than expected.
      for (auto& val : This->sel_) {
        val.loadTriggerResults(ev);
        ++(This->numberFound_);
      }
    }
    if (numberFound_ == 1) {
      return sel_[0].triggerResults();
    }
    if (numberFound_ == 0) {
      throw Exception(errors::ProductNotFound, "TooFewProducts")
        << "ProcessAndEventSelectors::getOneTriggerResults: "
        << "too few products found, exepcted one, got zero\n";
    }
    throw Exception(errors::ProductNotFound, "TooManyMatches")
      << "ProcessAndEventSelectors::getOneTriggerResults: "
      << "too many products found, expected one, got " << numberFound_ << '\n';
  }

  void
  ProcessAndEventSelectors::clearTriggerResults()
  {
    for (auto& val : sel_) {
      val.clearTriggerResults();
    }
    loadDone_ = false;
    numberFound_ = 0;
  }

  PVSentry::~PVSentry() { sel_.clearTriggerResults(); }

  PVSentry::PVSentry(ProcessAndEventSelectors& s) : sel_(s) {}

} // namespace art::detail
