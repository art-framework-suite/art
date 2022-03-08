#include "art/Framework/Core/Observer.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Event.h"
#include "art/Persistency/Provenance/PathSpec.h"
#include "art/Utilities/Globals.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"

#include <string>
#include <vector>

using namespace std;

using fhicl::ParameterSet;

using namespace art::detail;

namespace {

  std::optional<ProcessAndEventSelectors>
  make_selectors(vector<string> const& paths, std::string const& process_name)
  {
    if (empty(paths)) {
      return std::nullopt;
    }
    // Parse the event selection criteria into (process, trigger name
    // list) pairs.
    vector<pair<string, string>> PPS(paths.size());
    for (size_t i = 0; i < paths.size(); ++i) {
      PPS[i] = art::split_process_and_path_names(paths[i]);
    }
    return std::make_optional<ProcessAndEventSelectors>(PPS, process_name);
  }

  art::ProcessNameSelector const empty_process_name{""};
}

namespace art {

  Observer::~Observer() noexcept = default;

  Observer::Observer(ParameterSet const& pset)
    : Observer{pset.get<vector<string>>("SelectEvents", {}),
               pset.get<vector<string>>("RejectEvents", {})}
  {}

  Observer::Observer(vector<string> const& select_paths,
                     vector<string> const& reject_paths)
    : wantAllEvents_{empty(select_paths) and empty(reject_paths)}
    , process_name_{Globals::instance()->processName()}
    , selectors_{make_selectors(select_paths, process_name_)}
    , rejectors_{make_selectors(reject_paths, process_name_)}
  {}

  string const&
  Observer::processName() const
  {
    return process_name_;
  }

  bool
  Observer::wantEvent(ScheduleID const id, Event const& e) const
  {
    if (wantAllEvents_) {
      return true;
    }
    bool const select_event = selectors_ ? selectors_->matchEvent(id, e) : true;
    bool const reject_event =
      rejectors_ ? rejectors_->matchEvent(id, e) : false;
    return select_event and not reject_event;
  }

  Handle<TriggerResults>
  Observer::getTriggerResults(Event const& e) const
  {
    if (selectors_) {
      return selectors_->getOneTriggerResults(e);
    }

    // The following applies for cases where no SelectEvents entries
    // exist.
    Handle<TriggerResults> h;
    if (e.get(empty_process_name, h)) {
      return h;
    }
    return Handle<TriggerResults>{};
  }

} // namespace art
