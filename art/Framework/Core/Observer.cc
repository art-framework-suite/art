#include "art/Framework/Core/Observer.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/detail/parse_path_spec.h"
#include "art/Framework/Principal/Event.h"
#include "art/Utilities/Globals.h"
#include "art/Utilities/SharedResourcesRegistry.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"

#include <string>
#include <vector>

using namespace hep::concurrency;
using namespace std;

using fhicl::ParameterSet;

using namespace art::detail;

namespace {

  std::optional<ProcessAndEventSelectors>
  make_selectors(bool const want_all_events,
                 std::string const& process_name,
                 vector<string> const& paths)
  {
    if (want_all_events) {
      return std::nullopt;
    }
    auto const& triggerPathNames = art::Globals::instance()->triggerPathNames();
    // Parse the event selection criteria into (process, trigger name
    // list) pairs.
    vector<pair<string, string>> PPS(paths.size());
    for (size_t i = 0; i < paths.size(); ++i) {
      art::detail::parse_path_spec(paths[i], PPS[i]);
    }
    return std::make_optional<ProcessAndEventSelectors>(
      PPS, triggerPathNames, process_name);
  }

  art::ProcessNameSelector const empty_process_name{""};
}

namespace art {

  Observer::~Observer() noexcept = default;

  Observer::Observer(ParameterSet const& pset)
    : Observer{pset.get<vector<string>>("SelectEvents", {}), pset}
  {}

  Observer::Observer(vector<string> const& paths,
                     fhicl::ParameterSet const& pset)
    : wantAllEvents_{empty(paths)}
    , process_name_{Globals::instance()->processName()}
    , selectors_{make_selectors(wantAllEvents_, process_name_, paths)}
    , selector_config_id_{pset.id()}
  {}

  void
  Observer::registerProducts(ProductDescriptions&, ModuleDescription const&)
  {}

  void
  Observer::fillDescriptions(ModuleDescription const&)
  {}

  string const&
  Observer::processName() const
  {
    return process_name_;
  }

  bool
  Observer::wantEvent(Event const& e) const
  {
    if (wantAllEvents_) {
      return true;
    }
    bool const select_event = selectors_ and selectors_->matchEvent(e);
    return select_event;
  }

  fhicl::ParameterSetID
  Observer::selectorConfig() const
  {
    return selector_config_id_;
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
