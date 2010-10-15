#ifndef Framework_Actions_h
#define Framework_Actions_h


#include "fhiclcpp/ParameterSet.h"

#include <map>
#include <string>


namespace art {

  namespace actions {
    enum ActionCodes {
	IgnoreCompletely=0,
	Rethrow,
	SkipEvent,
	FailModule,
	FailPath,
	LastCode
    };

    const char* actionName(ActionCodes code);
  }  // namespace actions

  class ActionTable {
  public:
    typedef std::map<std::string, actions::ActionCodes> ActionMap;

    ActionTable();
    explicit ActionTable(const fhicl::ParameterSet&);
    ~ActionTable();

    void add(const std::string& category, actions::ActionCodes code);
    actions::ActionCodes find(const std::string& category) const;

  private:
    void addDefaults();
    ActionMap map_;
  };

}  // namespace art

#endif  // Framework_Actions_h
