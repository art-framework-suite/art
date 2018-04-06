#ifndef art_Framework_Principal_Actions_h
#define art_Framework_Principal_Actions_h
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/ActionCodes.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Sequence.h"

#include <map>
#include <string>
#include <vector>

namespace art {
  namespace actions {
    char const* actionName(ActionCodes code);
  }

  class ActionTable {
  public:
    ~ActionTable();
    ActionTable();

    struct Config {
      using Name = fhicl::Name;
      fhicl::Atom<bool> defaultExceptions{Name{"defaultExceptions"}, true};
      fhicl::Sequence<std::string> ignoreCompletely{Name{"IgnoreCompletely"},
                                                    {}};
      fhicl::Sequence<std::string> rethrow{Name{"Rethrow"}, {}};
      fhicl::Sequence<std::string> skipEvent{Name{"SkipEvent"}, {}};
      fhicl::Sequence<std::string> failModule{Name{"FailModule"}, {}};
      fhicl::Sequence<std::string> failPath{Name{"FailPath"}, {}};
    };

    explicit ActionTable(Config const&);

    ActionTable(ActionTable const&) = delete;
    ActionTable(ActionTable&&) = delete;
    ActionTable& operator=(ActionTable const&) = delete;
    ActionTable& operator=(ActionTable&&) = delete;

    // Accessors
    actions::ActionCodes find(std::string const& category) const;

    // Modifiers
    void add(std::string const& category, actions::ActionCodes);

  private:
    void addDefaults_();
    void install_(actions::ActionCodes, std::vector<std::string> const&);

    std::map<std::string, actions::ActionCodes> map_{};
  };
} // namespace art

#endif /* art_Framework_Principal_Actions_h */

// Local Variables:
// mode: c++
// End:
