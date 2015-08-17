#include "art/Persistency/Provenance/TypeTools.h"

#include "art/Utilities/DebugMacros.h"
#include "art/Utilities/Exception.h"
#include "boost/algorithm/string.hpp"
#include "boost/thread/tss.hpp"
#include "cetlib/container_algorithms.h"
#include "cetlib/demangle.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <algorithm>
#include <memory>
#include <regex>
#include <set>
#include <sstream>

#include "TClass.h"
#include "THashTable.h"
#include "TList.h"
#include "TBaseClass.h"

using namespace cet;
using namespace std;

namespace {
  typedef std::set<std::string> StringSet;
  StringSet & missingTypes() {
    static boost::thread_specific_ptr<StringSet> missingTypes_;
    if (0 == missingTypes_.get()) {
      missingTypes_.reset(new StringSet);
    }
    return *missingTypes_.get();
  }
}

namespace art {
  bool
  find_nested_type_named(string const& nested_type,
                         TClass * const type_to_search,
                         TypeWithDict & found_type)
  {
    assert(type_to_search != nullptr);
    found_type = TypeWithDict(std::string(type_to_search->GetName()) +
                              "::" +
                              nested_type);
    return bool(found_type);
  }

  bool
  value_type_of(TClass * const t, TypeWithDict & found_type)
  {
    return find_nested_type_named("value_type", t, found_type);
  }

  bool
  mapped_type_of(TClass * const t, TypeWithDict & found_type)
  {
    return find_nested_type_named("mapped_type", t, found_type);
  }

  void checkDictionaries(string const& name, bool recursive) {
    if (TClass::HasDictionarySelection(name.c_str())) {
      THashTable missing;
      TClass::GetClass(name.c_str())->GetMissingDictionaries(missing, recursive);
      if (missing.GetEntries()) {
        std::transform(missing.begin(),
                       missing.end(),
                       std::inserter(missingTypes(), missingTypes().begin()),
                       [](TObject * obj) { return dynamic_cast<TClass *>(obj)->GetName(); });
      }
    } else {
      missingTypes().insert(name);
    }
  }

  void reportFailedDictionaryChecks() {
    if (!missingTypes().empty()) {
      ostringstream ostr;
      for (StringSet::const_iterator it = missingTypes().begin(), itEnd = missingTypes().end();
           it != itEnd; ++it) {
        ostr << cet::demangle_symbol(*it) << "\n\n";
      }
      throw art::Exception(art::errors::DictionaryNotFound)
        << "No REFLEX data dictionary found for the following classes:\n\n"
        << ostr.str()
        << "Most likely each dictionary was never generated,\n"
        "but it may be that it was generated in the wrong package.\n"
        "Please add (or move) the specification\n"
        "<class name=\"whatever\"/>\n"
        "to the appropriate classes_def.xml file.\n"
        "If the class is a template instance, you may need\n"
        "to define a dummy variable of this type in classes.h.\n"
        "Also, if this class has any transient members,\n"
        "you need to specify them in classes_def.xml.";
    }
  }

  void public_base_classes(TClass * const cl,
                           vector<TClass *>& baseTypes) {
    if (cl == nullptr) {
      // throw
    }
    for (auto bobj : *(cl->GetListOfBases())){
      auto bb = dynamic_cast<TBaseClass *>(bobj);
      if (bb->Property() & kIsPublic) {
        baseTypes.push_back(bb->GetClassPointer());
      }
    }
  }

  TClass * type_of_template_arg(TClass * template_instance,
                                size_t arg_index)
  {
    if (template_instance == nullptr) {
      // throw;
    }
    TClass * result = nullptr;
    std::string type_result;
    std::string ti_name(template_instance->GetName());
    auto comma_count = 0ul;
    auto template_level = 0ul;
    auto pos = 0ul;
    auto arg_start = std::string::npos;
    while ((pos = ti_name.find_first_of("<>,", pos)) != std::string::npos) {
      switch(ti_name[pos]) {
      case '<':
        if ((++template_level == 1) && (arg_index == 0)) {
          arg_start = pos + 1;
        }
        break;
      case '>':
        if ((--template_level == 0ul) &&
            (comma_count == arg_index)) {
          type_result = ti_name.substr(arg_start, pos - arg_start);
        }
        break;
      case ',':
        if (template_level == 1ul) {
          if (comma_count == arg_index) {
            type_result = ti_name.substr(arg_start, pos - arg_start);
          } else if (++comma_count == arg_index) {
            arg_start = pos + 1;
          }
        }
        break;
      }
      ++pos;
    }
    if (!type_result.empty()) {
      result = TClass::GetClass(type_result.c_str());
    }
    return result;
  }

  bool is_instantiation_of(TClass * const cl,
                           std::string const &template_name)
  {
    if (cl == nullptr) {
      // throw;
    }
    return (std::string(cl->GetName()).find(template_name + "<") == 0ul);
  }

}  // art
