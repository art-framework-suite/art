#include "art/Framework/Core/ProcessAndEventSelectors.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/EventSelector.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Selector.h"
#include "canvas/Persistency/Common/TriggerResults.h"

#include <algorithm>
#include <cassert>
#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace cet;
using namespace std;
using namespace string_literals;

namespace art::detail {

  ProcessAndEventSelector::ProcessAndEventSelector(string const& nm,
                                                   EventSelector const& es)
    : processNameSelector_{nm}, eventSelector_{es}
  {}

  Handle<TriggerResults>
  ProcessAndEventSelector::triggerResults(Event const& e) const
  {
    Handle<TriggerResults> h;
    e.get(processNameSelector_, h);
    return h;
  }

  bool
  ProcessAndEventSelector::match(ScheduleID const id, Event const& e) const
  {
    auto h = triggerResults(e);
    return eventSelector_.acceptEvent(id, *h);
  }

  ProcessAndEventSelectors::ProcessAndEventSelectors(
    vector<pair<string, string>> const& path_specs,
    string const& process_name)
  {
    // Turn the passed path specs into a map of process name to
    // a vector of trigger names.
    map<string, vector<string>> paths_for_process;
    for (auto const& [proc_name, path_names] : path_specs) {
      auto const& pname = proc_name.empty() ? process_name : proc_name;
      paths_for_process[pname].push_back(path_names);
    }
    // Now go through all the process names found, and create an event
    // selector for each one.
    for (auto const& [pname, paths] : paths_for_process) {
      sel_.emplace_back(pname, EventSelector{paths});
    }
  }

  bool
  ProcessAndEventSelectors::matchEvent(ScheduleID const id,
                                       Event const& e) const
  {
    assert(not empty(sel_));
    return std::any_of(
      begin(sel_), end(sel_), [id, &e](auto& val) { return val.match(id, e); });
  }

  Handle<TriggerResults>
  ProcessAndEventSelectors::getOneTriggerResults(Event const& ev) const
  {
    for (auto& val : sel_) {
      if (auto h = val.triggerResults(ev); h.isValid()) {
        return h;
      }
    }
    return Handle<TriggerResults>{};
  }

} // namespace art::detail
