#include "art/Framework/Core/EventObserverBase.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "art/Framework/Core/detail/parse_path_spec.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"
#include "hep_concurrency/SerialTaskQueueChain.h"

#include <string>
#include <vector>

using namespace hep::concurrency;
using namespace std;

using fhicl::ParameterSet;

namespace art {

EventObserverBase::
~EventObserverBase()
{
}

EventObserverBase::
EventObserverBase(vector<string> const& paths, fhicl::ParameterSet const& pset)
  : ModuleBase()
  , wantAllEvents_{false}
  , selectors_{}
  , process_name_{}
  , selector_config_id_{pset.id()}
{
  init_(paths);
}

EventObserverBase::
EventObserverBase(ParameterSet const& pset)
  : ModuleBase()
  , wantAllEvents_{false}
  , selectors_{}
  , process_name_{}
  , selector_config_id_{pset.id()}
{
  auto const& paths = pset.get<vector<string>>("SelectEvents", {});
  init_(paths);
}

void
EventObserverBase::
init_(vector<string> const& paths)
{
  ServiceHandle<TriggerNamesService const> TNS;
  process_name_ = TNS->getProcessName();
  auto const& trigPaths = TNS->getTrigPaths();
  if (paths.empty()) {
    // No event selection criteria given, we want all events.
    wantAllEvents_ = true;
    selectors_.setupDefault(trigPaths);
    return;
  }
  // Parse the event selection criteria into
  // (process, trigger name list) pairs.
  vector<pair<string, string>> PPS(paths.size());
  for (size_t i = 0; i < paths.size(); ++i) {
    detail::parse_path_spec(paths[i], PPS[i]);
  }
  selectors_.setup(PPS, trigPaths, process_name_);
}

void
EventObserverBase::
registerProducts(ProductDescriptions&, ModuleDescription const&)
{
}

string const&
EventObserverBase::
processName() const
{
  return process_name_;
}

bool
EventObserverBase::
wantAllEvents() const
{
  return wantAllEvents_;
}

bool
EventObserverBase::
wantEvent(Event const& e)
{
  return selectors_.wantEvent(e);
}

fhicl::ParameterSetID
EventObserverBase::
selectorConfig() const
{
  return selector_config_id_;
}

Handle<TriggerResults>
EventObserverBase::
getTriggerResults(Event const& e) const
{
  return selectors_.getOneTriggerResults(e);
}

detail::CachedProducts&
EventObserverBase::
cachedProducts()
{
  return selectors_;
}

} // namespace art
