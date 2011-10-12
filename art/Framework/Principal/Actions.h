#ifndef art_Framework_Principal_Actions_h
#define art_Framework_Principal_Actions_h

#include "art/Framework/Principal/fwd.h"

#include <map>
#include <string>

namespace fhicl {
  class ParameterSet;
}

// ----------------------------------------------------------------------

namespace art {
  namespace actions {
    const char * actionName(ActionCodes code);
  }  // actions
}

class art::ActionTable {
public:


  ActionTable();
  explicit ActionTable(const fhicl::ParameterSet &);

  // use compiler-generated copy c'tor, copy assignment, and d'tor

  void add(const std::string & category, actions::ActionCodes code);
  actions::ActionCodes find(const std::string & category) const;

private:
  typedef std::map<std::string, actions::ActionCodes> ActionMap;

  void addDefaults_();
  void install_(actions::ActionCodes code, const fhicl::ParameterSet & scheduler);

  ActionMap map_;
};  // ActionTable

#endif /* art_Framework_Principal_Actions_h */

// Local Variables:
// mode: c++
// End:
