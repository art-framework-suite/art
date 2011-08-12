#ifndef art_Framework_Principal_Actions_h
#define art_Framework_Principal_Actions_h

#include "art/Framework/Principal/fwd.h"

#include "fhiclcpp/ParameterSet.h"

#include <map>
#include <string>

// ----------------------------------------------------------------------

namespace art {
  namespace actions {
    const char* actionName(ActionCodes code);
  }  // actions
}

class art::ActionTable {
public:
  typedef std::map<std::string, actions::ActionCodes> ActionMap;

  ActionTable();
  explicit ActionTable(const fhicl::ParameterSet&);

  // use compiler-generated copy c'tor, copy assignment, and d'tor

  void add(const std::string& category, actions::ActionCodes code);
  actions::ActionCodes find(const std::string& category) const;

private:
  void addDefaults();
  ActionMap map_;
};  // ActionTable

#endif /* art_Framework_Principal_Actions_h */

// Local Variables:
// mode: c++
// End:
