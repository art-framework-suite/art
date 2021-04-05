#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Persistency/Provenance/PathSpec.h"
#include "fhiclcpp/ParameterSet.h"

// vim: set sw=2 expandtab :

#include <cstddef>
#include <limits>
#include <string>
#include <vector>

using namespace std;
using art::detail::entry_selector_t;
using fhicl::ParameterSet;

namespace {
  constexpr auto invalid_entry = std::numeric_limits<size_t>::max();
  entry_selector_t
  for_(art::PathID const id)
  {
    return [id](art::PathSpec const& spec) { return spec.path_id == id; };
  }
  entry_selector_t
  for_(std::string const& name)
  {
    return [&name](art::PathSpec const& spec) { return spec.name == name; };
  }
}

namespace art {

  TriggerNamesService::TriggerNamesService(
    vector<PathSpec> const& triggerPathSpecs,
    string const& processName,
    ParameterSet const& physicsPSet,
    ActivityRegistry& registry)
    : triggerPathSpecs_{triggerPathSpecs}, processName_{processName}
  {
    for (auto const& spec_str : triggerPathSpecs) {
      triggerPathNames_.push_back(spec_str.name);
      moduleNames_.push_back(physicsPSet.get<vector<string>>(spec_str.name));
    }
    registry.sPostOpenFile.watch(this,
                                 &TriggerNamesService::updateTriggerInfo_);
  }

  void
  TriggerNamesService::updateTriggerInfo_(std::string const&)
  {}

  string const&
  TriggerNamesService::getProcessName() const
  {
    return processName_;
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
  TriggerNamesService::getTrigPath(PathID const id) const
  {
    auto const i = index_for(id);
    if (i == invalid_entry) {
      throw Exception{errors::OtherArt}
        << "A path name could not be found corresponding to path ID "
        << to_string(id) << '\n';
    }
    return triggerPathSpecs_[i].name;
  }

  PathID
  TriggerNamesService::findTrigPath(string const& name) const
  {
    auto const i = index_(for_(name));
    if (i == invalid_entry) {
      return PathID::invalid();
    }
    return triggerPathSpecs_[i].path_id;
  }

  vector<string> const&
  TriggerNamesService::getTrigPathModules(string const& name) const
  {
    return moduleNames_.at(index_(for_(name)));
  }

  vector<string> const&
  TriggerNamesService::getTrigPathModules(PathID const id) const
  {
    return moduleNames_.at(index_for(id));
  }

  string const&
  TriggerNamesService::getTrigPathModule(string const& name,
                                         size_t const j) const
  {
    return getTrigPathModules(name).at(j);
  }

  string const&
  TriggerNamesService::getTrigPathModule(PathID const id, size_t const j) const
  {
    return getTrigPathModules(id).at(j);
  }

  size_t
  TriggerNamesService::index_for(PathID const id) const
  {
    return index_(for_(id));
  }

  size_t
  TriggerNamesService::index_(entry_selector_t matched_entry) const
  {
    auto const b = begin(triggerPathSpecs_);
    auto const e = end(triggerPathSpecs_);

    auto it = std::find_if(b, e, matched_entry);
    if (it == e) {
      return invalid_entry;
    }
    return std::distance(b, it);
  }

} // namespace art
