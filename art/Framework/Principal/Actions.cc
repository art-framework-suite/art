#include "art/Framework/Principal/Actions.h"

#include "canvas/Utilities/DebugMacros.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include <iostream>
#include <vector>

using namespace cet;
using namespace std;
using fhicl::ParameterSet;

namespace art {
  namespace actions {
    namespace {
      struct ActionNames {
        ActionNames() : table_(LastCode + 1)
        {
          table_[IgnoreCompletely] = "IgnoreCompletely";
          table_[Rethrow] = "Rethrow";
          table_[SkipEvent] = "SkipEvent";
          table_[FailModule] = "FailModule";
          table_[FailPath] = "FailPath";
        }

        using Table = vector<char const*>;
        Table table_;
      };
    }

    char const*
    actionName(ActionCodes const code)
    {
      static ActionNames tab;
      return static_cast<unsigned int>(code) < tab.table_.size() ?
               tab.table_[code] :
               "UnknownAction";
    }
  }

  ActionTable::ActionTable() : map_() { addDefaults_(); }

  ActionTable::ActionTable(ParameterSet const& scheduleOpts) : map_()
  {
    if (scheduleOpts.get<bool>("defaultExceptions", true)) {
      addDefaults_();
    }
    install_(actions::SkipEvent, scheduleOpts);
    install_(actions::Rethrow, scheduleOpts);
    install_(actions::IgnoreCompletely, scheduleOpts);
    install_(actions::FailModule, scheduleOpts);
    install_(actions::FailPath, scheduleOpts);
  }

  void
  ActionTable::addDefaults_()
  {
    // This is where defaults that are not 'Rethrow' would be populated.
    if (2 > debugit())
      return;

    for (auto const& pr : map_) {
      cerr << pr.first << ',' << pr.second << '\n';
    }
    cerr << endl;
  }

  void
  ActionTable::install_(actions::ActionCodes const code,
                        ParameterSet const& scheduler)
  {
    auto const& action_names =
      scheduler.get<vector<string>>(actionName(code), {});
    for_all(action_names, [this, code](auto const& action_name) {
      this->add(action_name, code);
    });
  }

  void
  ActionTable::add(string const& category, actions::ActionCodes const code)
  {
    map_[category] = code;
  }

  actions::ActionCodes
  ActionTable::find(string const& category) const
  {
    auto it = map_.find(category);
    return it != end(map_) ? it->second : actions::Rethrow;
  }

} // art
