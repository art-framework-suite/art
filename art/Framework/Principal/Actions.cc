#include "art/Framework/Principal/Actions.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/ActionCodes.h"
//#include "boost/lambda/lambda.hpp"
#include "canvas/Utilities/DebugMacros.h"
#include "canvas/Utilities/Exception.h"
//#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"

#include <algorithm>
#include <cstddef>
#include <map>
#include <string>
#include <vector>

using namespace cet;
using namespace std;

using fhicl::ParameterSet;

namespace art {

namespace actions {

char const*
actionName(ActionCodes code)
{
  vector<const char*>
  names{
      "IgnoreCompletely"
    , "Rethrow"
    , "SkipEvent"
    , "FailModule"
    , "FailPath"
  };
  return (static_cast<size_t>(code) < names.size()) ? names[code] : "UnknownAction";
}

} // namespace actions

ActionTable::
~ActionTable()
{
}

ActionTable::
ActionTable()
  : map_()
{
  addDefaults_();
}

ActionTable::
ActionTable(const ParameterSet& scheduleOpts)
  : map_()
{
  if (scheduleOpts.get<bool>("defaultExceptions", true)) {
    addDefaults_();
  }
  install_(actions::IgnoreCompletely, scheduleOpts);
  install_(actions::Rethrow, scheduleOpts);
  install_(actions::SkipEvent, scheduleOpts);
  install_(actions::FailModule, scheduleOpts);
  install_(actions::FailPath, scheduleOpts);
}

void
ActionTable::
addDefaults_()
{
  map_[Exception::codeToString(errors::ProductNotFound)] = actions::SkipEvent;
  map_[Exception::codeToString(errors::InvalidReference)] = actions::SkipEvent;
  map_[Exception::codeToString(errors::NullPointerError)] = actions::SkipEvent;
  map_[Exception::codeToString(errors::EventTimeout)] = actions::SkipEvent;
  map_[Exception::codeToString(errors::DataCorruption)] = actions::SkipEvent;
  map_[Exception::codeToString(errors::NotFound)] = actions::SkipEvent;
}

void
ActionTable::
install_(actions::ActionCodes code, const ParameterSet& scheduler)
{
  //using namespace boost::lambda;
  vector<string> v(scheduler.get<vector<string>>(actions::actionName(code), vector<string>()));
  //for_all(v, var(map_)[boost::lambda::_1] = code);
  for_each(v.begin(), v.end(), [this, code](auto const& val) mutable { map_[val] = code; });
}

void
ActionTable::
add(const string& category, actions::ActionCodes code)
{
  map_[category] = code;
}

actions::ActionCodes
ActionTable::
find(string const& category) const
{
  auto I = map_.find(category);
  return (I != map_.end()) ? I->second : actions::Rethrow;
}

} // namespace art

