#ifndef art_Framework_Principal_Actions_h
#define art_Framework_Principal_Actions_h

#include "art/Framework/Principal/fwd.h"

#include <map>
#include <string>

namespace fhicl {
  class ParameterSet;
}

namespace art {
  namespace actions {
    char const* actionName(ActionCodes code);
  } // actions
}

class art::ActionTable {
public:
  ActionTable();
  explicit ActionTable(fhicl::ParameterSet const&);

  void add(std::string const& category, actions::ActionCodes code);
  actions::ActionCodes find(std::string const& category) const;

private:
  using ActionMap = std::map<std::string, actions::ActionCodes>;

  void addDefaults_();
  void install_(actions::ActionCodes code,
                fhicl::ParameterSet const& scheduler);

  ActionMap map_;
};

#endif /* art_Framework_Principal_Actions_h */

// Local Variables:
// mode: c++
// End:
