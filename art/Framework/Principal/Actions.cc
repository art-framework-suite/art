#include "art/Framework/Principal/Actions.h"

#include "art/Utilities/DebugMacros.h"
#include "art/Utilities/Exception.h"
#include "boost/lambda/lambda.hpp"
#include "cetlib/container_algorithms.h"
#include <iostream>
#include <vector>


using namespace cet;
using namespace std;
using fhicl::ParameterSet;

namespace art {
  namespace actions {
    namespace {
      struct ActionNames
      {
        ActionNames():table_(LastCode + 1)
        {
          table_[IgnoreCompletely]="IgnoreCompletely";
          table_[Rethrow]="Rethrow";
          table_[SkipEvent]="SkipEvent";
          table_[FailModule]="FailModule";
          table_[FailPath]="FailPath";
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
    addDefaults();
  }

  namespace {
    inline void install(actions::ActionCodes code,
                        ActionTable::ActionMap& out,
                        const ParameterSet& pset)
    {
      using namespace boost::lambda;
      typedef vector<string> vstring;

      // we cannot have parameters in the main process section so look
      // for an untracked (optional) ParameterSet called
      // "services.options" for now.  Notice that all exceptions (most
      // actally) throw art::Exception with the configuration category.
      // This category should probably be more specific or a derived
      // exception type should be used so the catch can be more
      // specific.

//      cerr << pset.toString() << endl;

      ParameterSet defopts;
      ParameterSet services = pset.get<ParameterSet>("services", ParameterSet());
      ParameterSet opts =
        services.get<ParameterSet>("scheduler", defopts);
      //cerr << "looking for " << actionName(code) << endl;
      vstring v =
        opts.get<vector<string> >(actionName(code),vstring());
      for_all(v, var(out)[boost::lambda::_1] = code);

    }
  }

  ActionTable::ActionTable(const ParameterSet& pset) : map_()
  {
    addDefaults();

    install(actions::SkipEvent, map_, pset);
    install(actions::Rethrow, map_, pset);
    install(actions::IgnoreCompletely, map_, pset);
    install(actions::FailModule, map_, pset);
    install(actions::FailPath, map_, pset);
  }

  void ActionTable::addDefaults()
  {
    using namespace boost::lambda;
    // populate defaults that are not 'Rethrow'
    // 'Rethrow' is the default default.
    map_[art::Exception::codeToString(errors::ProductNotFound)]=
      actions::SkipEvent;
    map_[art::Exception::codeToString(errors::InvalidReference)]=
      actions::SkipEvent;
    map_[art::Exception::codeToString(errors::NullPointerError)]=
      actions::SkipEvent;
    map_[art::Exception::codeToString(errors::EventTimeout)]=
      actions::SkipEvent;
    map_[art::Exception::codeToString(errors::DataCorruption)]=
      actions::SkipEvent;
    map_[art::Exception::codeToString(errors::NotFound)]=
      actions::SkipEvent;

    if(2 <= debugit())
      {
        ActionMap::const_iterator ib(map_.begin()),ie(map_.end());
        for(;ib!=ie;++ib)
          cerr << ib->first << ',' << ib->second << '\n';
        cerr << endl;
      }

  }

  void ActionTable::add(const string& category,
                        actions::ActionCodes code)
  {
    map_[category] = code;
  }

  actions::ActionCodes ActionTable::find(const string& category) const
  {
    ActionMap::const_iterator i(map_.find(category));
    return i!=map_.end() ? i->second : actions::Rethrow;
  }

}  // art
