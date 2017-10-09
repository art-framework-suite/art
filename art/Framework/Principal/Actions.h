#ifndef art_Framework_Principal_Actions_h
#define art_Framework_Principal_Actions_h
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/ActionCodes.h"

#include <map>
#include <string>

namespace fhicl {

  class ParameterSet;

} // namespace fhicl

namespace art {
  namespace actions {

    const char* actionName(ActionCodes code);

  } // namespace actions

  class ActionTable {

  public: // MEMBER FUNCTIONS -- Special Member Functions
    ~ActionTable();

    ActionTable();

    explicit ActionTable(const fhicl::ParameterSet&);

    ActionTable(ActionTable const&) = delete;

    ActionTable(ActionTable&&) = delete;

    ActionTable& operator=(ActionTable const&) = delete;

    ActionTable& operator=(ActionTable&&) = delete;

  public: // MEMBER FUNCTIONS -- Public API, accessors
    actions::ActionCodes find(std::string const& category) const;

  public: // MEMBER FUNCTIONS -- Public API, modifiers
    void add(std::string const& category, actions::ActionCodes);

  private: // MEMBER FUNCTIONS -- Implementation details
    void addDefaults_();

    void install_(actions::ActionCodes, fhicl::ParameterSet const&);

  private:
    std::map<std::string, actions::ActionCodes> map_;
  };

} // namespace art

#endif /* art_Framework_Principal_Actions_h */

// Local Variables:
// mode: c++
// End:
