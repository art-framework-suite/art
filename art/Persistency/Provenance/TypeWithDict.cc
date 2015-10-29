#include "art/Persistency/Provenance/TypeWithDict.h"
// vim: set sw=2:

#include "art/Utilities/Exception.h"
#include "art/Utilities/FriendlyName.h"
#include "art/Utilities/uniform_type_name.h"
#include "tbb/concurrent_unordered_map.h"
#include "TClass.h"
#include "TDataType.h"
#include "TDictionary.h"
#include "TEnum.h"
#include "TROOT.h"

using namespace art;
using namespace std;

namespace {

void
throwIfUnsupportedType(string const& uname)
{
  string reason;
  static string const constSuffix = " const";
  static string const constPrefix = "const ";
  static auto const constSz = constSuffix.size();
  auto endChar = uname.back();
  switch (endChar) {
    case '&':
      reason = "reference";
      break;
    case '*':
      reason = "pointer";
      break;
    case ']':
      reason = "array";
      break;
  }
  if ((uname.size() > constSz) &&
      ((uname.substr(0, constSz) == constPrefix) ||
       (uname.substr(uname.size() - constSz) == constSuffix))) {
    reason = "const";
  }
  if (reason.empty()) {
    return;
  }
  throw Exception(errors::UnimplementedFeature)
      << "Attempted to access ROOT type information for type\n"
      << uname
      << "\nwhich is not supported by art due to its being a "
      << reason
      << ".\n";
}

} // unnamed namespace

namespace art {

void
TypeWithDict::
print(ostream& os) const
{
  switch (category_) {
    case Category::NONE:
      os << "NONE";
      break;
    case Category::ENUMTYPE:
      os << "Enumerated type "
         << dynamic_cast<TEnum*>(tDict_)->GetName();
      break;
    default:
      if (!bool(id_)) {
        throw Exception(errors::LogicError)
            << "No typeid information for type of category "
            << to_string(category_)
            << ".\n";
      }
      id_.print(os);
      break;
  }
}

char const*
TypeWithDict::
name() const
{
  char const* result = "";
  switch (category_) {
    case Category::NONE:
      break;
    case Category::ENUMTYPE:
      break;
    default:
      if (!bool(id_)) {
        throw Exception(errors::LogicError)
            << "No typeid information for type of category "
            << to_string(category_)
            << ".\n";
      }
      result = id_.name();
      break;
  }
  return result;
}

string
TypeWithDict::
className() const
{
  string result;
  switch (category_) {
    case Category::NONE:
      result = "NONE";
      break;
    case Category::ENUMTYPE:
      result = dynamic_cast<TEnum*>(tDict_)->GetName();
      break;
    default:
      if (!bool(id_)) {
        throw Exception(errors::LogicError)
            << "No typeid information for type of category "
            << to_string(category_)
            << ".\n";
      }
      result = id_.className();
      break;
  }
  return result;
}

string
TypeWithDict::
friendlyClassName() const
{
  string result;
  switch (category_) {
    case Category::NONE:
      result = "NONE";
      break;
    case Category::ENUMTYPE:
      result = friendlyname::friendlyName(
                 dynamic_cast<TEnum*>(tDict_)->GetName());
      break;
    default:
      if (!bool(id_)) {
        throw Exception(errors::LogicError)
            << "No typeid information for type of category "
            << to_string(category_)
            << ".\n";
      }
      result = id_.friendlyClassName();
      break;
  }
  return result;
}

TClass*
TypeWithDict::
tClass() const
{
  if (category_ == Category::CLASSTYPE) {
    return dynamic_cast<TClass*>(tDict_);
  }
  throw Exception(errors::TypeConversion)
      << "TypeWithDict: requested operation requires category: "
      << to_string(Category::CLASSTYPE)
      << ".\n"
      << "The category of this type is actually: "
      << to_string(category_)
      << ".\n";
}

TEnum*
TypeWithDict::
tEnum() const
{
  if (category_ == Category::ENUMTYPE) {
    return dynamic_cast<TEnum*>(tDict_);
  }
  throw Exception(errors::TypeConversion)
      << "TypeWithDict: requested operation requires category: "
      << to_string(Category::ENUMTYPE)
      << ".\n"
      << "The category of this type is actually: "
      << to_string(category_)
      << ".\n";
}

TDataType*
TypeWithDict::
tDataType() const
{
  if (category_ == Category::BASICTYPE) {
    return dynamic_cast<TDataType*>(tDict_);
  }
  throw Exception(errors::TypeConversion)
      << "TypeWithDict: requested operation requires category: "
      << to_string(Category::BASICTYPE)
      << ".\n"
      << "The category of this type is actually: "
      << to_string(category_)
      << ".\n";
}

TDictionary*
TypeWithDict::
dictFromTypeInfo_(type_info const& ti)
{
  throwIfUnsupportedType(uniform_type_name(ti));
  TDictionary* result = TClass::GetClass(ti);
  if (result == nullptr) {
    result = TDataType::GetDataType(TDataType::GetType(ti));
  }
  if (result == nullptr) {
    result = TEnum::GetEnum(ti, TEnum::kAutoload);
  }
  if ((result == nullptr) && (ti == typeid(void))) {
    result = gROOT->GetType("void");
  }
  if (result != nullptr) {
    return result;
  }
  throw Exception(errors::DictionaryNotFound)
      << "TypeWithDict::dictFromTypeInfo_: no dictionary found for type\n"
      << uniform_type_name(ti)
      << ".\n"
      << "Please ensure that it is properly specified in classes_def.xml,\n"
      << "with the correct header inclusions and template instantiations\n"
      << "(if appropriate) in classes.h. If this was a transient data "
      "member,\n"
      << "ensure that it is properly marked as such in classes_def.xml\n";
}

TDictionary*
TypeWithDict::
dictFromName_(string const& name)
{
  TDictionary* result = nullptr;
  static tbb::concurrent_unordered_map<string, TDictionary*> s_nameToDict;
  auto I = s_nameToDict.find(name);
  if (I != s_nameToDict.end()) {
    result = I->second;
    return result;
  }
  string uname = uniform_type_name(name);
  throwIfUnsupportedType(uname);
  result = TClass::GetClass(uname.c_str());
  if (result == nullptr) {
    result = TEnum::GetEnum(uname.c_str(), TEnum::kAutoload);
  }
  if (result == nullptr) {
    // Note that we are constrained to looking in the list of what are
    // nominally typedefs, not fundamental types. However ROOT adds
    // entries for what *it* classifies as fundamental types (See the
    // enum EDataType in core/meta/inc/TDataType.h for the list).
    result = gROOT->GetType(uname.c_str());
  }
  if (result != nullptr) {
    s_nameToDict.emplace(name, result);
  }
  return result;
}

TypeWithDict::Category
TypeWithDict::
categoryFromDict_(TDictionary* tDict)
{
  Category result = Category::NONE;
  if (tDict == nullptr) {
    return result;
  }
  if (dynamic_cast<TClass*>(tDict) != nullptr) {
    result = Category::CLASSTYPE;
    return result;
  }
  if (dynamic_cast<TDataType*>(tDict) != nullptr) {
    result = Category::BASICTYPE;
    return result;
  }
  if (dynamic_cast<TEnum*>(tDict) != nullptr) {
    result = Category::ENUMTYPE;
    return result;
  }
  throw Exception(errors::LogicError)
      << "INTENRAL ERROR: TypeWithDict::categoryFromDict_ encountered a type "
      << tDict->GetName()
      << " of unknown category "
      << tDict->IsA()->GetName()
      << ".\n";
}

TypeID
TypeWithDict::
typeIDFromDictAndCategory_(TDictionary* tDict, Category category)
{
  TypeID result;
  if (category == Category::NONE) {
    return result;
  }
  if (category == Category::ENUMTYPE) {
    // Can't get typeID from TEnum.
    return result;
  }
  if (category == Category::CLASSTYPE) {
    result = TypeID(*dynamic_cast<TClass*>(tDict)->GetTypeInfo());
    return result;
  }
  if (category == Category::BASICTYPE) {
    type_info const* t = nullptr;
    auto type = dynamic_cast<TDataType*>(tDict)->GetType();
    if ((type == kOther_t/*-1*/) || (type == kNoType_t/*0*/) ||
        (type == kCounter/*6*/) || (type == kBits/*15*/) ||
        (type >= kNumDataTypes/*23*/)) {
      throw Exception(errors::LogicError)
          << "INTERNAL ERROR: TypeWithDict::typeIDFromDictAndCategory_ "
          << "encountered unknown type "
          << type
          << " corresponding to a "
          << dynamic_cast<TDataType*>(tDict)->GetTypeName()
          << ".\n";
    }
    switch (type) {
      case kChar_t: // 1
        t = &typeid(char);
        break;
      case kShort_t: // 2
        t = &typeid(short);
        break;
      case kInt_t: // 3
        t = &typeid(int);
        break;
      case kLong_t: // 4
        t = &typeid(long);
        break;
      case kFloat_t: // 5
        t = &typeid(float);
        break;
      case kCharStar: // 7
        t = &typeid(char*);
        break;
      case kDouble_t: // 8
        t = &typeid(double);
        break;
      case kDouble32_t: // 9
        t = &typeid(Double32_t);
        break;
      case kchar: // 10
        t = &typeid(char);
        break;
      case kUChar_t: // 11
        t = &typeid(unsigned char);
        break;
      case kUShort_t: // 12
        t = &typeid(unsigned short);
        break;
      case kUInt_t: // 13
        t = &typeid(unsigned int);
        break;
      case kULong_t: // 14
        t = &typeid(unsigned long);
        break;
      case kLong64_t: // 16
        t = &typeid(long long);
        break;
      case kULong64_t: // 17
        t = &typeid(unsigned long long);
        break;
      case kBool_t: // 18
        t = &typeid(bool);
        break;
      case kFloat16_t: // 19
        t = &typeid(Float16_t);
        break;
      case kVoid_t: // 20
        t = &typeid(void);
        break;
      case kDataTypeAliasUnsigned_t: // 21
        t = &typeid(unsigned int);
        break;
      case kDataTypeAliasSignedChar_t: // 22
        t = &typeid(signed char);
        break;
    }
    result = TypeID(*t);
    return result;
  }
  throw Exception(errors::LogicError)
      << "INTERNAL ERROR: typeIDFromDictAndCategory_: "
      << "Unknown type category "
      << static_cast<unsigned short>(category)
      << " ("
      << to_string(category)
      << ").\n";
}

std::type_info const&
TypeWithDict::
typeInfo() const
{
  if (!*this) {
    throw Exception(errors::LogicError)
        << "TypeWithDict: typeInfo() requested on an invalid object\n.";
  }
  return id_.typeInfo();
}

string
to_string(TypeWithDict::Category category)
{
  string result;
  if (category == TypeWithDict::Category::NONE) {
    result = "NONE";
    return result;
  }
  if (category == TypeWithDict::Category::BASICTYPE) {
    result = "BASICTYPE";
    return result;
  }
  if (category == TypeWithDict::Category::CLASSTYPE) {
    result = "CLASSTYPE";
    return result;
  }
  if (category == TypeWithDict::Category::ENUMTYPE) {
    result = "ENUMTYPE";
    return result;
  }
  throw Exception(errors::LogicError)
      << "INTENRAL ERROR: to_string(TypeWithDict::Category): "
      << "Cannot interpret category: "
      << static_cast<unsigned short>(category)
      << "\n";
}

} // namespace art

