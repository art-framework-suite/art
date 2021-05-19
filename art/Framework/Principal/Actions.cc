#include "art/Framework/Principal/Actions.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/ActionCodes.h"
#include "canvas/Utilities/DebugMacros.h"
#include "cetlib/container_algorithms.h"

using namespace cet;
using namespace std;

namespace art {

  namespace actions {

    char const*
    actionName(ActionCodes code)
    {
      vector<const char*> names{
        "IgnoreCompletely", "Rethrow", "SkipEvent", "FailModule", "FailPath"};
      return (static_cast<size_t>(code) < names.size()) ? names[code] :
                                                          "UnknownAction";
    }

  } // namespace actions

  ActionTable::~ActionTable() = default;

  ActionTable::ActionTable() { addDefaults_(); }

  ActionTable::ActionTable(Config const& c)
  {
    if (c.defaultExceptions()) {
      addDefaults_();
    }
    install_(actions::IgnoreCompletely, c.ignoreCompletely());
    install_(actions::Rethrow, c.rethrow());
    install_(actions::SkipEvent, c.skipEvent());
    install_(actions::FailModule, c.failModule());
    install_(actions::FailPath, c.failPath());
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
  ActionTable::install_(actions::ActionCodes code,
                        vector<string> const& action_names)
  {
    for_all(action_names, [this, code](auto const& action_name) {
      this->add(action_name, code);
    });
  }

  void
  ActionTable::add(const string& category, actions::ActionCodes code)
  {
    map_[category] = code;
  }

  actions::ActionCodes
  ActionTable::find(string const& category) const
  {
    auto I = map_.find(category);
    return (I != map_.end()) ? I->second : actions::Rethrow;
  }

} // namespace art
