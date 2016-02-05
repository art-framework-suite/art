#include "canvas/Persistency/Provenance/TypeTools.h"
// vim: set sw=2:

#include "canvas/Persistency/Provenance/TypeWithDict.h"
#include "canvas/Utilities/DebugMacros.h"
#include "canvas/Utilities/Exception.h"
#include "boost/algorithm/string.hpp"
#include "boost/thread/tss.hpp"
#include "cetlib/container_algorithms.h"
#include "cetlib/demangle.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "TBaseClass.h"
#include "TClass.h"
#include "TDataMember.h"
#include "TDictAttributeMap.h"
#include "TEnum.h"
#include "THashTable.h"
#include "TList.h"
#include "TROOT.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <regex>
#include <set>
#include <sstream>

using namespace cet;
using namespace std;

namespace {

set<string>&
missingTypes()
{
  static boost::thread_specific_ptr<set<string>> missingTypes_;
  if (missingTypes_.get() == nullptr) {
    missingTypes_.reset(new set<string>);
  }
  return *missingTypes_.get();
}

string
template_arg_at(string const& name, unsigned long desired_arg)
{
  string retval;
  auto comma_count = 0ul;
  auto template_level = 0ul;
  auto arg_start = string::npos;
  auto pos = 0ul;
  pos = name.find_first_of("<,>", pos);
  while (pos != string::npos) {
    if (name[pos] == '<') {
      ++template_level;
      if ((desired_arg == 0ul) && (template_level == 1ul)) {
        // Found the begin of the desired template arg.
        arg_start = pos + 1;
      }
    }
    else if (name[pos] == '>') {
      --template_level;
      if ((desired_arg == comma_count) && (template_level == 0ul)) {
        // Found the end of the desired template arg.
        retval = name.substr(arg_start, pos - arg_start);
        return retval;
      }
    }
    else {
      // We have a comma.
      if (template_level == 1ul) {
        // Ignore arguments not at the first level.
        if (comma_count == desired_arg) {
          // Found the end of the desired template arg.
          retval = name.substr(arg_start, pos - arg_start);
          return retval;
        }
        ++comma_count;
        if (comma_count == desired_arg) {
          // Found the begin of the desired template arg.
          arg_start = pos + 1;
        }
      }
    }
    ++pos;
    pos = name.find_first_of("<,>", pos);
  }
  return retval;
}

} // unnamed namespace

namespace art {

bool
find_nested_type_named(string const& nested_type, TClass* const type_to_search,
                       TypeWithDict& found_type)
{
  if (type_to_search == nullptr) {
    throw Exception(errors::NullPointerError, "find_nested_type_named: ")
        << "Null TClass pointer passed for type_to_search!\n";
  }
  found_type = TypeWithDict(std::string(type_to_search->GetName()) + "::" +
                            nested_type);
  return bool(found_type);
}

bool
value_type_of(TClass* const t, TypeWithDict& found_type)
{
  return find_nested_type_named("value_type", t, found_type);
}

bool
mapped_type_of(TClass* const t, TypeWithDict& found_type)
{
  return find_nested_type_named("mapped_type", t, found_type);
}

void
checkDictionaries(string const& name_orig, bool recursive/*=false*/,
                  int level/*=0*/)
{
  static set<string> checked_names;
  //string indent(level * 2, ' ');
  string name(name_orig);
  //cout << indent << "Checking dictionary for: " << name << endl;
  // Strip leading const.
  if (name.size() > 6) {
    if (!name.compare(0, 6, "const ")) {
      name.erase(0, 6);
    }
  }
  // Strip trailing const.
  if (name.size() > 6) {
    if (!name.compare(name.size() - 6, 6, "const ")) {
      name.erase(name.size() - 6, 6);
    }
  }
  //
  // FIXME: What about volatile & restrict?
  //
  // Strip trailing &&.
  if (name.size() > 2) {
    if (!name.compare(name.size() - 2, 2, "&&")) {
      name.erase(name.size() - 2, 2);
    }
  }
  // Strip trailing &.
  if (name.size() > 1) {
    if (name[name.size()-1] == '&') {
      name.erase(name.size() - 1, 1);
    }
  }
  if (name.empty()) {
    return;
  }
  // Strip trailing *.
  {
    auto pos = name.size();
    while ((pos > 0) && (name[pos-1] == '*')) {
        --pos;
    }
    if (pos == 0) {
      // Name was all '*'.
      return;
    }
    name.erase(pos);
  }
  // Strip leading std::
  if (!name.compare(0, 5, "std::")) {
    if (name.size() == 5) {
      // Name is nothing but "std::".
      return;
    }
    name.erase(0, 5);
  }
  {
    auto I = checked_names.find(name);
    if (I != checked_names.end()) {
      // Already checked this name.
      //cout << indent << "type already checked" << endl;
      return;
    }
    checked_names.insert(name);
  }
  if (!name.compare("void")) {
    //cout << indent << "type is void" << endl;
    return;
  }
  if (name.size() > 12) {
    if (!name.compare(name.size() - 13, 13, "::(anonymous)")) {
      //cout << indent << "type is actually an anonymous namespace name" << endl;
      return;
    }
  }
  TypeWithDict ty(name);
  if (ty) {
    if (ty.category() == TypeWithDict::Category::NONE) {
      //cout << "category: " << "NONE" << endl;
      throw Exception(errors::LogicError, "checkDictionaries: ")
          << "Type category of name is NONE: "
          << name
          << 'n';
    }
    if (ty.category() == TypeWithDict::Category::CLASSTYPE) {
      //cout << "category: " << "CLASSTYPE" << endl;
    }
    if (ty.category() == TypeWithDict::Category::ENUMTYPE) {
      //cout << "category: " << "ENUMTYPE" << endl;
      //cout << indent << "type is an enumeration" << endl;
      return;
    }
    if (ty.category() == TypeWithDict::Category::BASICTYPE) {
      //cout << "category: " << "BASICTYPE" << endl;
      //cout << indent << "type is basic" << endl;
      return;
    }
  }
  auto cl = TClass::GetClass(name.c_str());
  if (cl == nullptr) {
    missingTypes().insert(name);
    // Note: The rest of the code assumes cl is not nullptr.
    return;
  }
  if (!TClass::HasDictionarySelection(name.c_str())) {
    missingTypes().insert(name);
  }
  {
    auto am = cl->GetAttributeMap();
    if (am && am->HasKey("persistent") &&
        am->GetPropertyAsString("persistent") == std::string("false")) {
      // Marked transient in the selection xml.
      //cout << indent << "class marked not persistent in selection xml" << endl;
      return;
    }
    if (am && am->HasKey("transient") &&
        am->GetPropertyAsString("transient") == std::string("true")) {
      // Marked transient in the selection xml.
      //cout << indent << "class marked transient in selection xml" << endl;
      return;
    }
  }
#if 0
  {
    THashTable missing;
    cl->GetMissingDictionaries(missing, recursive);
    TClass::GetClass(name.c_str())->GetMissingDictionaries(missing, recursive);
    if (missing.GetEntries()) {
      std::transform(missing.begin(), missing.end(), std::inserter(missingTypes(),
      missingTypes().begin()), [](TObject * obj) {
        return dynamic_cast<TClass*>(obj)->GetName();
      });
    }
  }
#endif // 0
  {
    static std::regex const reNoSplit("^(art::PtrVector(<|Base$)|art::Assns<)");
    if (std::regex_search(name, reNoSplit)) {
      FDEBUG(1)
          << "Setting NoSplit on class "
          << name
          << "\n";
      //cout << indent << "Setting NoSplit on class " << cl->GetName() << endl;
      cl->SetCanSplit(0);
    }
  }
  if (!recursive) {
    return;
  }
  if (!name.compare(0, 6, "array<")) {
    auto arg0 = template_arg_at(name, 0);
    if (arg0.empty()) {
      throw Exception(errors::LogicError, "checkDictionaries: ")
          << "Could not get first template arg from: "
          << name
          << '\n';
    }
    name = arg0;
    checkDictionaries(arg0, true, level + 2);
    return;
  }
  if (!name.compare(0, 6, "deque<")) {
    auto arg0 = template_arg_at(name, 0);
    if (arg0.empty()) {
      throw Exception(errors::LogicError, "checkDictionaries: ")
          << "Could not get first template arg from: "
          << name
          << '\n';
    }
    checkDictionaries(arg0, true, level + 2);
    return;
  }
  if (!name.compare(0, 13, "forward_list<")) {
    auto arg0 = template_arg_at(name, 0);
    if (arg0.empty()) {
      throw Exception(errors::LogicError, "checkDictionaries: ")
          << "Could not get first template arg from: "
          << name
          << '\n';
    }
    checkDictionaries(arg0, true, level + 2);
    return;
  }
  if (!name.compare(0, 5, "list<")) {
    auto arg0 = template_arg_at(name, 0);
    if (arg0.empty()) {
      throw Exception(errors::LogicError, "checkDictionaries: ")
          << "Could not get first template arg from: "
          << name
          << '\n';
    }
    checkDictionaries(arg0, true, level + 2);
    return;
  }
  if (!name.compare(0, 6, "string")) {
    // Ignore, root has special handling for this.
    return;
  }
  if (!name.compare(0, 9, "u16string")) {
    // Ignore, root has special handling for this.
    // FIXME: It does not!
    return;
  }
  if (!name.compare(0, 9, "u32string")) {
    // Ignore, root has special handling for this.
    // FIXME: It does not!
    return;
  }
  if (!name.compare(0, 7, "wstring")) {
    // Ignore, root has special handling for this.
    // FIXME: It does not!
    return;
  }
  if (!name.compare(0, 13, "basic_string<")) {
    auto arg0 = template_arg_at(name, 0);
    if (arg0.empty()) {
      throw Exception(errors::LogicError, "checkDictionaries: ")
          << "Could not get first template arg from: "
          << name
          << '\n';
    }
    checkDictionaries(arg0, true, level + 2);
    return;
  }
  if (!name.compare(0, 7, "vector<")) {
    auto arg0 = template_arg_at(name, 0);
    if (arg0.empty()) {
      throw Exception(errors::LogicError, "checkDictionaries: ")
          << "Could not get first template arg from: "
          << name
          << '\n';
    }
    checkDictionaries(arg0, true, level + 2);
    return;
  }
  if (!name.compare(0, 4, "map<")) {
    auto arg0 = template_arg_at(name, 0);
    if (arg0.empty()) {
      throw Exception(errors::LogicError, "checkDictionaries: ")
          << "Could not get first template arg from: "
          << name
          << '\n';
    }
    checkDictionaries(arg0, true, level + 2);
    auto arg1 = template_arg_at(name, 1);
    if (arg1.empty()) {
      throw Exception(errors::LogicError, "checkDictionaries: ")
          << "Could not get second template arg from: "
          << name
          << '\n';
    }
    checkDictionaries(arg1, true, level + 2);
    //FIXME: Should check Compare, and Allocator too!
    return;
  }
  if (!name.compare(0, 9, "multimap<")) {
    auto arg0 = template_arg_at(name, 0);
    if (arg0.empty()) {
      throw Exception(errors::LogicError, "checkDictionaries: ")
          << "Could not get first template arg from: "
          << name
          << '\n';
    }
    checkDictionaries(arg0, true, level + 2);
    auto arg1 = template_arg_at(name, 1);
    if (arg1.empty()) {
      throw Exception(errors::LogicError, "checkDictionaries: ")
          << "Could not get second template arg from: "
          << name
          << '\n';
    }
    checkDictionaries(arg1, true, level + 2);
    //FIXME: Should check Compare, and Allocator too!
    return;
  }
  if (!name.compare(0, 14, "unordered_map<")) {
    auto arg0 = template_arg_at(name, 0);
    if (arg0.empty()) {
      throw Exception(errors::LogicError, "checkDictionaries: ")
          << "Could not get first template arg from: "
          << name
          << '\n';
    }
    checkDictionaries(arg0, true, level + 2);
    auto arg1 = template_arg_at(name, 1);
    if (arg1.empty()) {
      throw Exception(errors::LogicError, "checkDictionaries: ")
          << "Could not get second template arg from: "
          << name
          << '\n';
    }
    checkDictionaries(arg1, true, level + 2);
    //FIXME: Should check Hash, Pred, and Allocator too!
    return;
  }
  if (!name.compare(0, 19, "unordered_multimap<")) {
    auto arg0 = template_arg_at(name, 0);
    if (arg0.empty()) {
      throw Exception(errors::LogicError, "checkDictionaries: ")
          << "Could not get first template arg from: "
          << name
          << '\n';
    }
    checkDictionaries(arg0, true, level + 2);
    auto arg1 = template_arg_at(name, 1);
    if (arg1.empty()) {
      throw Exception(errors::LogicError, "checkDictionaries: ")
          << "Could not get second template arg from: "
          << name
          << '\n';
    }
    checkDictionaries(arg1, true, level + 2);
    //FIXME: Should check Hash, Pred, and Allocator too!
    return;
  }
  if (!name.compare(0, 4, "set<")) {
    auto arg0 = template_arg_at(name, 0);
    if (arg0.empty()) {
      throw Exception(errors::LogicError, "checkDictionaries: ")
          << "Could not get first template arg from: "
          << name
          << '\n';
    }
    checkDictionaries(arg0, true, level + 2);
    //FIXME: Should check Compare, and Allocator too!
    return;
  }
  if (!name.compare(0, 9, "multiset<")) {
    auto arg0 = template_arg_at(name, 0);
    if (arg0.empty()) {
      throw Exception(errors::LogicError, "checkDictionaries: ")
          << "Could not get first template arg from: "
          << name
          << '\n';
    }
    checkDictionaries(arg0, true, level + 2);
    //FIXME: Should check Compare, and Allocator too!
    return;
  }
  if (!name.compare(0, 14, "unordered_set<")) {
    auto arg0 = template_arg_at(name, 0);
    if (arg0.empty()) {
      throw Exception(errors::LogicError, "checkDictionaries: ")
          << "Could not get first template arg from: "
          << name
          << '\n';
    }
    checkDictionaries(arg0, true, level + 2);
    //FIXME: Should check Hash, Pred, and Allocator too!
    return;
  }
  if (!name.compare(0, 19, "unordered_multiset<")) {
    auto arg0 = template_arg_at(name, 0);
    if (arg0.empty()) {
      throw Exception(errors::LogicError, "checkDictionaries: ")
          << "Could not get first template arg from: "
          << name
          << '\n';
    }
    checkDictionaries(arg0, true, level + 2);
    //FIXME: Should check Hash, Pred, and Allocator too!
    return;
  }
  if (!name.compare(0, 6, "queue<")) {
    auto arg0 = template_arg_at(name, 0);
    if (arg0.empty()) {
      throw Exception(errors::LogicError, "checkDictionaries: ")
          << "Could not get first template arg from: "
          << name
          << '\n';
    }
    checkDictionaries(arg0, true, level + 2);
    //FIXME: Should check Container too!
    return;
  }
  if (!name.compare(0, 15, "priority_queue<")) {
    auto arg0 = template_arg_at(name, 0);
    if (arg0.empty()) {
      throw Exception(errors::LogicError, "checkDictionaries: ")
          << "Could not get first template arg from: "
          << name
          << '\n';
    }
    checkDictionaries(arg0, true, level + 2);
    //FIXME: Should check Container, and Compare too!
    return;
  }
  if (!name.compare(0, 6, "stack<")) {
    auto arg0 = template_arg_at(name, 0);
    if (arg0.empty()) {
      throw Exception(errors::LogicError, "checkDictionaries: ")
          << "Could not get first template arg from: "
          << name
          << '\n';
    }
    checkDictionaries(arg0, true, level + 2);
    //FIXME: Should check Container too!
    return;
  }
  for (auto obj : *cl->GetListOfBases()) {
    auto bc = dynamic_cast<TBaseClass*>(obj);
    //cout << indent << "  Base class: " << bc->GetName() << endl;
    checkDictionaries(bc->GetName(), true, level + 2);
  }
  for (auto obj : *cl->GetListOfDataMembers()) {
    auto dm = dynamic_cast<TDataMember*>(obj);
    //cout << indent << "  Data member: " << dm->GetName() << endl;
    if (!dm->IsPersistent()) {
      // The data member comment in the header file starts with '!'.
      //cout << indent << "  marked not persistent in header file" << endl;
      continue;
    }
    if (dm->Property() & kIsStatic) {
      // Static data member.
      //cout << indent << "  static" << endl;
      continue;
    }
    auto am = dm->GetAttributeMap();
    if (am && am->HasKey("persistent") &&
        am->GetPropertyAsString("persistent") == std::string("false")) {
      // Marked transient in the selection xml.
      //cout << indent << "  marked not persistent in selection xml" << endl;
      continue;
    }
    if (am && am->HasKey("transient") &&
        am->GetPropertyAsString("transient") == std::string("true")) {
      // Marked transient in the selection xml.
      //cout << indent << "  marked transient in selection xml" << endl;
      continue;
    }
    if (am && am->HasKey("comment") &&
        (am->GetPropertyAsString("comment")[0] == '!')) {
      // Marked transient in the selection xml.
      //cout << indent << "  commented transient in selection xml" << endl;
      continue;
    }
    checkDictionaries(dm->GetTrueTypeName(), true, level + 2);
  }
}

void
reportFailedDictionaryChecks()
{
  if (missingTypes().empty()) {
    return;
  }
  ostringstream ostr;
  for (auto I = missingTypes().cbegin(), E = missingTypes().cend();
       I != E; ++I) {
    ostr << "     " << cet::demangle_symbol(*I) << "\n";
  }
  throw Exception(errors::DictionaryNotFound)
      << "No dictionary found for the following classes:\n\n"
      << ostr.str()
      << "\nMost likely they were never generated, but it may be that they "
         "were generated in the wrong package.\n"
         "\n"
         "Please add (or move) the specification\n"
         "\n"
         "     <class name=\"MyClassName\"/>\n"
         "\n"
         "to the appropriate classes_def.xml file.\n"
         "\n"
         "If the class is a template instance, you may need\n"
         "to define a dummy variable of this type in classes.h.\n"
         "\n"
         "Also, if this class has any transient members,\n"
         "you need to specify them in classes_def.xml.";
}

void
public_base_classes(TClass* const cl, vector<TClass*>& baseTypes)
{
  if (cl == nullptr) {
    throw Exception(errors::NullPointerError, "public_base_classes: ")
        << "Null TClass pointer passed!\n";
  }
  for (auto bobj : *cl->GetListOfBases()) {
    auto bb = dynamic_cast<TBaseClass*>(bobj);
    if (bb->Property() & kIsPublic) {
      baseTypes.push_back(bb->GetClassPointer());
    }
  }
}

TClass*
type_of_template_arg(TClass* template_instance, size_t desired_arg)
{
  if (template_instance == nullptr) {
    throw Exception(errors::NullPointerError, "type_of_template_arg: ")
        << "Null TClass pointer passed!\n";
  }
  TClass* result = nullptr;
  std::string ti_name(template_instance->GetName());
  auto comma_count = 0ul;
  auto template_level = 0ul;
  auto arg_start = std::string::npos;
  auto pos = 0ul;
  pos = ti_name.find_first_of("<>,", pos);
  while (pos != std::string::npos) {
    switch (ti_name[pos]) {
      case '<':
        ++template_level;
        if ((desired_arg == 0ul) && (template_level == 1ul)) {
          // Found the begin of the desired template arg.
          arg_start = pos + 1;
        }
        break;
      case '>':
        --template_level;
        if ((desired_arg == comma_count) && (template_level == 0ul)) {
          // Found the end of the desired template arg.
          auto type_result = ti_name.substr(arg_start, pos - arg_start);
          result = TClass::GetClass(type_result.c_str());
          //Note: A nullptr result is allowed now.
          //if (result == nullptr) {
          //  throw Exception(errors::DictionaryNotFound,
          //                  "type_of_template_arg: ")
          //    << "Could not get TClass for template parameter: "
          //    << type_result
          //    << '\n';
          //}
          return result;
        }
        break;
      case ',':
        if (template_level != 1ul) {
          // Ignore arguments not at the first level.
          break;
        }
        if (comma_count == desired_arg) {
          // Found the end of the desired template arg.
          auto type_result = ti_name.substr(arg_start, pos - arg_start);
          result = TClass::GetClass(type_result.c_str());
          //Note: A nullptr result is allowed now.
          //if (result == nullptr) {
          //  throw Exception(errors::DictionaryNotFound,
          //                  "type_of_template_arg: ")
          //    << "Could not get TClass for template parameter: "
          //    << type_result
          //    << '\n';
          //}
          return result;
        }
        ++comma_count;
        if (comma_count == desired_arg) {
          // Found the begin of the desired template arg.
          arg_start = pos + 1;
        }
        break;
    }
    ++pos;
    pos = ti_name.find_first_of("<>,", pos);
  }
  return result;
}

bool
is_instantiation_of(TClass* const cl, std::string const& template_name)
{
  if (cl == nullptr) {
    throw Exception(errors::NullPointerError, "is_instantiation_of: ")
        << "Null TClass pointer passed!\n";
  }
  return std::string(cl->GetName()).find(template_name + "<") == 0ul;
}

} // namespace art

