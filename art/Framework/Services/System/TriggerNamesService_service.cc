// ======================================================================
//
// TriggerNamesService
//
// ======================================================================

#include "art/Framework/Services/System/TriggerNamesService.h"

#include "canvas/Persistency/Common/TriggerResults.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSetRegistry.h"

using art::TriggerNamesService;
using namespace cet;
using namespace fhicl;
using namespace std;

// ----------------------------------------------------------------------

TriggerNamesService::TriggerNamesService(ParameterSet const& procPS,
                                         Strings const& trigger_path_names)
  :
  trignames_{trigger_path_names},
  process_name_{procPS.get<string>("process_name")},
  wantSummary_{procPS.get<bool>("services.scheduler.wantSummary", false)}
{
  // Make and hold onto a parameter set for posterity.
  trigger_pset_.put("trigger_paths",  trignames_);
  ParameterSetRegistry::put(trigger_pset_);

  auto const& physics = procPS.get<ParameterSet>("physics", {});
  end_names_ = physics.get<Strings>("end_paths", {});

  auto assign_position = [](auto& posmap, size_type const i, auto const& name) {
    posmap[name] = i;
  };

  using namespace std::placeholders;
  cet::for_all_with_index(trignames_, std::bind(assign_position, std::ref(trigpos_), _1, _2));
  cet::for_all_with_index(end_names_, std::bind(assign_position, std::ref(end_pos_), _1, _2));

  cet::transform_all(trignames_,
                     std::back_inserter(modulenames_),
                     [&physics](std::string const& par){
                       return physics.get<Strings>(par);
                     });
}  // c'tor

// ----------------------------------------------------------------------

bool
TriggerNamesService::getTrigPaths(TriggerResults const& triggerResults,
                                  Strings& trigPaths) const
{
  ParameterSet pset;
  if (!ParameterSetRegistry::get(triggerResults.parameterSetID(), pset)) {
    return false;
  }
  auto tmpPaths = pset.get<Strings>("trigger_paths", {});
  if (tmpPaths.size() != triggerResults.size()) {
    throw art::Exception(art::errors::Unknown)
      << "TriggerNamesService::getTrigPaths, Trigger names vector and\n"
      "TriggerResults are different sizes.  This should be impossible,\n"
      "please send information to reproduce this problem to\n"
      "the ART developers.\n";
  }
  std::swap(tmpPaths, trigPaths);
  return true;
}
