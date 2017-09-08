#include "art/Framework/Principal/Actions.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/ActionCodes.h"
#include "canvas/Utilities/DebugMacros.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"

#include <algorithm>
#include <cstddef>
#include <map>
#include <string>
#include <vector>

using namespace cet;
using namespace std;

using fhicl::ParameterSet;

namespace art {

  namespace actions {

    char const*
    actionName(ActionCodes code)
    {
      vector<const char*>
        names{
        "IgnoreCompletely"
          , "Rethrow"
          , "SkipEvent"
          , "FailModule"
          , "FailPath"
          };
      return (static_cast<size_t>(code) < names.size()) ? names[code] : "UnknownAction";
    }

  } // namespace actions

  ActionTable::
  ~ActionTable()
  {
  }

  ActionTable::
  ActionTable()
    : map_()
  {
    addDefaults_();
  }

  ActionTable::
  ActionTable(const ParameterSet& scheduleOpts)
    : map_()
  {
    if (scheduleOpts.get<bool>("defaultExceptions", true)) {
      addDefaults_();
    }
    install_(actions::IgnoreCompletely, scheduleOpts);
    install_(actions::Rethrow, scheduleOpts);
    install_(actions::SkipEvent, scheduleOpts);
    install_(actions::FailModule, scheduleOpts);
    install_(actions::FailPath, scheduleOpts);
  }

  void
  ActionTable::
  addDefaults_()
  {
    // This is where defaults that are not 'Rethrow' would be populated.
    if (2 > debugit()) return;

    for (auto const& pr : map_) {
      cerr << pr.first << ',' << pr.second << '\n';
    }
    cerr << endl;
  }

  void
  ActionTable::
  install_(actions::ActionCodes code, const ParameterSet& scheduler)
  {
    auto const& action_names = scheduler.get<vector<string>>(actionName(code), {});
    for_all(action_names, [this, code](auto const& action_name) { this->add(action_name, code); });
  }

  void
  ActionTable::
  add(const string& category, actions::ActionCodes code)
  {
    map_[category] = code;
  }

  actions::ActionCodes
  ActionTable::
  find(string const& category) const
  {
    auto I = map_.find(category);
    return (I != map_.end()) ? I->second : actions::Rethrow;
  }

} // namespace art
