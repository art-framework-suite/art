#include "art/Framework/Core/Observer.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/detail/parse_path_spec.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Utilities/Globals.h"
#include "art/Utilities/SharedResourcesRegistry.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"

#include <string>
#include <vector>

using namespace hep::concurrency;
using namespace std;

using fhicl::ParameterSet;

namespace art {

  Observer::~Observer() noexcept = default;

  Observer::Observer(vector<string> const& paths,
                     fhicl::ParameterSet const& pset)
    : selector_config_id_{pset.id()}
  {
    init_(paths);
  }

  Observer::Observer(ParameterSet const& pset) : selector_config_id_{pset.id()}
  {
    auto const& paths = pset.get<vector<string>>("SelectEvents", {});
    init_(paths);
  }

  void
  Observer::init_(vector<string> const& paths)
  {
    process_name_ = Globals::instance()->processName();
    auto const& triggerPathNames = Globals::instance()->triggerPathNames();
    if (paths.empty()) {
      // No event selection criteria given, we want all events.
      wantAllEvents_ = true;
      selectors_.setupDefault(triggerPathNames);
      return;
    }
    // Parse the event selection criteria into
    // (process, trigger name list) pairs.
    vector<pair<string, string>> PPS(paths.size());
    for (size_t i = 0; i < paths.size(); ++i) {
      detail::parse_path_spec(paths[i], PPS[i]);
    }
    selectors_.setup(PPS, triggerPathNames, process_name_);
  }

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
  Observer::wantAllEvents() const
  {
    return wantAllEvents_;
  }

  bool
  Observer::wantEvent(Event const& e)
  {
    return selectors_.wantEvent(e);
  }

  fhicl::ParameterSetID
  Observer::selectorConfig() const
  {
    return selector_config_id_;
  }

  Handle<TriggerResults>
  Observer::getTriggerResults(Event const& e) const
  {
    return selectors_.getOneTriggerResults(e);
  }

  detail::ProcessAndEventSelectors&
  Observer::processAndEventSelectors()
  {
    return selectors_;
  }

} // namespace art
