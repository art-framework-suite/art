#include "art/Framework/Core/EventObserver.h"

#include "art/Framework/Core/detail/OutputModuleUtils.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"

#include <string>
#include <vector>

using namespace std;

using fhicl::ParameterSet;

namespace art {

  EventObserver::
  EventObserver(fhicl::OptionalTable<EOConfig> const& config)
    : wantAllEvents_(false)
    , selectors_()
    , process_name_()
    , selector_config_id_()
  {
    init_( config.get_PSet() );
  }

  EventObserver::
  EventObserver(ParameterSet const& pset)
    : wantAllEvents_(false)
    , selectors_()
    , process_name_()
    , selector_config_id_()
  {
    init_( pset.get<ParameterSet>("SelectEvents", {}) );
  }

  void EventObserver::init_(fhicl::ParameterSet const& SE)
  {
    ServiceHandle<TriggerNamesService> TNS;
    process_name_ = TNS->getProcessName();
    vector<string> const& trigPaths = TNS->getTrigPaths();
    selector_config_id_ = SE.id();
    if (SE.is_empty()) {
      // No event selection criteria given, we want all events.
      wantAllEvents_ = true;
      selectors_.setupDefault(trigPaths);
      return;
    }
    vector<string> PS = SE.get<vector<string>>("SelectEvents");
    if (PS.empty()) {
      // No event selection criteria given, we want all events.
      wantAllEvents_ = true;
      selectors_.setupDefault(trigPaths);
      return;
    }
    // Parse the event selection criteria
    // into (process, trigger name list) pairs.
    vector<pair<string, string>> PPS(PS.size());
    for (size_t i = 0; i < PS.size(); ++i) {
      detail::parse_path_spec(PS[i], PPS[i]);
    }
    selectors_.setup(PPS, trigPaths, process_name_);
  }

} // namespace art
