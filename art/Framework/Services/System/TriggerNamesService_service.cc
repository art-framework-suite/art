#include "art/Framework/Services/System/TriggerNamesService.h"
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Common/TriggerResults.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "cetlib_except/exception.h"
#include "fhiclcpp/ParameterSetRegistry.h"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

using namespace std;
using namespace cet;
using namespace fhicl;
using namespace hep::concurrency;

namespace art {

  TriggerNamesService::TriggerNamesService(
    vector<string> const& triggerPathNames,
    string const& processName,
    ParameterSet const& triggerPSet,
    ParameterSet const& physicsPSet)
    : triggerPathNames_{triggerPathNames}
    , processName_{processName}
    , triggerPSet_{triggerPSet}
  {
    size_t i{0};
    for (auto const& pathname : triggerPathNames_) {
      trigPathNameToTrigBitPos_[pathname] = i++;
      moduleNames_.push_back(physicsPSet.get<vector<string>>(pathname));
    }
  }

  string const&
  TriggerNamesService::getProcessName() const
  {
    return processName_;
  }

  ParameterSet const&
  TriggerNamesService::getTriggerPSet() const
  {
    return triggerPSet_;
  }

  vector<string> const&
  TriggerNamesService::getTrigPaths() const
  {
    return triggerPathNames_;
  }

  size_t
  TriggerNamesService::size() const
  {
    return triggerPathNames_.size();
  }

  string const&
  TriggerNamesService::getTrigPath(size_t const i) const
  {
    return triggerPathNames_.at(i);
  }

  size_t
  TriggerNamesService::find(map<string, size_t> const& posmap,
                            string const& name) const
  {
    auto const I = posmap.find(name);
    if (I == posmap.cend()) {
      return posmap.size();
    }
    return I->second;
  }

  size_t
  TriggerNamesService::findTrigPath(string const& name) const
  {
    return find(trigPathNameToTrigBitPos_, name);
  }

  vector<string> const&
  TriggerNamesService::getTrigPathModules(string const& name) const
  {
    return moduleNames_.at(find(trigPathNameToTrigBitPos_, name));
  }

  vector<string> const&
  TriggerNamesService::getTrigPathModules(size_t const i) const
  {
    return moduleNames_.at(i);
  }

  string const&
  TriggerNamesService::getTrigPathModule(string const& name,
                                         size_t const j) const
  {
    return moduleNames_.at(find(trigPathNameToTrigBitPos_, name)).at(j);
  }

  string const&
  TriggerNamesService::getTrigPathModule(size_t const i, size_t const j) const
  {
    return moduleNames_.at(i).at(j);
  }

} // namespace art
