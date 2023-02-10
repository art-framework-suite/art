#include "art/Framework/Principal/Actions.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/ActionCodes.h"
#include "canvas/Utilities/DebugMacros.h"
#include "cetlib/container_algorithms.h"

namespace art {

  ActionTable::~ActionTable() = default;

  ActionTable::ActionTable()
  {
    addDefaults_();
  }

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

    for (auto const& [name, code] : map_) {
      std::cerr << name << ',' << code << '\n';
    }
    std::cerr << '\n';
  }

  void
  ActionTable::install_(actions::ActionCodes const code,
                        std::vector<std::string> const& action_names)
  {
    cet::for_all(action_names, [this, code](auto const& action_name) {
      add(action_name, code);
    });
  }

  void
  ActionTable::add(std::string const& category, actions::ActionCodes const code)
  {
    map_[category] = code;
  }

  actions::ActionCodes
  ActionTable::find(std::string const& category) const
  {
    auto I = map_.find(category);
    return (I != map_.end()) ? I->second : actions::Rethrow;
  }

} // namespace art
