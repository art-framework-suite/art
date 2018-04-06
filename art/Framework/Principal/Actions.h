#ifndef art_Framework_Principal_Actions_h
#define art_Framework_Principal_Actions_h
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/ActionCodes.h"

#include <map>
#include <string>

namespace fhicl {
  class ParameterSet;
}

namespace art {
  namespace actions {
    char const* actionName(ActionCodes code);
  }

  class ActionTable {
  public:
    ~ActionTable();
    ActionTable();

    explicit ActionTable(fhicl::ParameterSet const&);

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
    void install_(actions::ActionCodes, fhicl::ParameterSet const&);

    std::map<std::string, actions::ActionCodes> map_{};
  };
} // namespace art

#endif /* art_Framework_Principal_Actions_h */

// Local Variables:
// mode: c++
// End:
