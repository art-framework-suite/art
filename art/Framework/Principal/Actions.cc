#include "art/Framework/Principal/Actions.h"

#include "art/Utilities/DebugMacros.h"
#include "art/Utilities/Exception.h"
#include "boost/lambda/lambda.hpp"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include <iostream>
#include <vector>

using namespace cet;
using namespace std;
using fhicl::ParameterSet;

namespace art {
  namespace actions {
    namespace {
      struct ActionNames {
        ActionNames(): table_(LastCode + 1) {
          table_[IgnoreCompletely] = "IgnoreCompletely";
          table_[Rethrow] = "Rethrow";
          table_[SkipEvent] = "SkipEvent";
          table_[FailModule] = "FailModule";
          table_[FailPath] = "FailPath";
        }

        typedef vector<const char*> Table;
        Table table_;
      };
    }

    const char* actionName(ActionCodes code)
    {
      static ActionNames tab;
      return static_cast<unsigned int>(code) < tab.table_.size() ?
             tab.table_[code] : "UnknownAction";
    }
  }

  ActionTable::ActionTable() : map_()
  {
    addDefaults_();
  }

  ActionTable::ActionTable(const ParameterSet& scheduleOpts) :
    map_()
  {
    addDefaults_();
    install_(actions::SkipEvent, scheduleOpts);
    install_(actions::Rethrow, scheduleOpts);
    install_(actions::IgnoreCompletely, scheduleOpts);
    install_(actions::FailModule, scheduleOpts);
    install_(actions::FailPath, scheduleOpts);
  }

  void ActionTable::addDefaults_()
  {
    using namespace boost::lambda;
    // populate defaults that are not 'Rethrow'
    // 'Rethrow' is the default default.
    map_[art::Exception::codeToString(errors::ProductNotFound)] =
      actions::SkipEvent;
    map_[art::Exception::codeToString(errors::InvalidReference)] =
      actions::SkipEvent;
    map_[art::Exception::codeToString(errors::NullPointerError)] =
      actions::SkipEvent;
    map_[art::Exception::codeToString(errors::EventTimeout)] =
      actions::SkipEvent;
    map_[art::Exception::codeToString(errors::DataCorruption)] =
      actions::SkipEvent;
    map_[art::Exception::codeToString(errors::NotFound)] =
      actions::SkipEvent;
    if (2 <= debugit()) {
      ActionMap::const_iterator ib(map_.begin()), ie(map_.end());
      for (; ib != ie; ++ib)
      { cerr << ib->first << ',' << ib->second << '\n'; }
      cerr << endl;
    }
  }

  void
  ActionTable::install_(actions::ActionCodes code,
                        const ParameterSet& scheduler)
  {
    using namespace boost::lambda;
    typedef vector<string> vstring;
    vstring v(scheduler.get<vector<string> >(actionName(code), vstring()));
    for_all(v, var(map_)[boost::lambda::_1] = code);
  }

  void ActionTable::add(const string& category,
                        actions::ActionCodes code)
  {
    map_[category] = code;
  }

  actions::ActionCodes ActionTable::find(const string& category) const
  {
    ActionMap::const_iterator i(map_.find(category));
    return i != map_.end() ? i->second : actions::Rethrow;
  }

}  // art
