#include "art/Persistency/Provenance/TypeWithDict.h"

#include "art/Utilities/Exception.h"
#include "art/Utilities/FriendlyName.h"
#include "art/Utilities/uniform_type_name.h"

#include "tbb/concurrent_unordered_map.h"

#include "TClass.h"
#include "TDataType.h"
#include "TDictionary.h"
#include "TEnum.h"
#include "TROOT.h"

namespace {
  void throwForUnexpectedCategory(art::TypeWithDict::Category category)
  {
    throw art::Exception(art::errors::LogicError)
      << "INTERNAL ERROR: a TypeWithDict function was unable to deal with category "
      << static_cast<unsigned short>(category)
      << " (" << to_string(category) << ").\n";
  }

  void throwDictionaryNotFound(std::string const & wherefrom,
                               std::string const & typeName) {
    throw art::Exception(art::errors::DictionaryNotFound)
      << wherefrom << ": no data dictionary found for type\n"
      << typeName << ".\n"
      << "Please ensure that it is properly specified in classes_def.xml,\n"
      << "with the correct header inclusions and template instantiations\n"
      << "(if appropriate) in classes.h. If this was a transient data member,\n"
      << "ensure that it is properly marked as such in classes_def.xml\n";
  }

  void throwDictionaryNotFound(std::string const & wherefrom,
                               std::type_info const & tid) {
    throwDictionaryNotFound(wherefrom, art::uniform_type_name(tid));
  }

  void throwIfUnsupportedType(std::string const & uname) {
    std::string reason;
    static std::string const constSuffix = " const";
    static std::string const constPrefix = "const ";
    static auto const constSz = constSuffix.size();
    auto endChar = uname.back();
    switch(endChar) {
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
    if (!reason.empty()) {
      throw art::Exception(art::errors::UnimplementedFeature)
        << "Attempted to access ROOT type information for type\n"
        << uname
        << "\nwhich is not supported by art due to its being a "
        << reason
        << ".\n";
    }
  }
}

void
art::TypeWithDict::
print(std::ostream & os) const
{
  switch (category_) {
  case Category::NONE:
    os << "NONE";
  case Category::ENUMTYPE:
    os << "Enumerated type "
       << dynamic_cast<TEnum *>(tDict_)->GetName();
  default:
    if (bool(id_)) {
      id_.print(os);
    } else {
      throw Exception(errors::LogicError)
        << "No typeid information for type of category "
        << to_string(category_)
        << ".\n";
    }
  }
}

char const *
art::TypeWithDict::
name() const
{
  char const * result = "";
  switch (category_) {
  case Category::NONE:
  case Category::ENUMTYPE:
    break;
  default:
    if (bool(id_)) {
      result = id_.name();
    } else {
      throw Exception(errors::LogicError)
        << "No typeid information for type of category "
        << to_string(category_)
        << ".\n";
    }
  }
  return result;
}

std::string
art::TypeWithDict::
className() const
{
  std::string result;
  switch (category_) {
  case Category::NONE:
    result = "NONE";
  case Category::ENUMTYPE:
    result = dynamic_cast<TEnum *>(tDict_)->GetName();
  default:
    if (bool(id_)) {
      result = id_.className();
    } else {
      throw Exception(errors::LogicError)
        << "No typeid information for type of category "
        << to_string(category_)
        << ".\n";
    }
  }
  return result;
}

std::string
art::TypeWithDict::
friendlyClassName() const
{
  std::string result;
  switch (category_) {
  case Category::NONE:
    result = "NONE";
  case Category::ENUMTYPE:
    result = friendlyname::friendlyName(dynamic_cast<TEnum *>(tDict_)->GetName());
  default:
    if (bool(id_)) {
      result = id_.className();
    } else {
      throw Exception(errors::LogicError)
        << "No typeid information for type of category "
        << to_string(category_)
        << ".\n";
    }
  }
  return result;
}

TClass *
art::TypeWithDict::
tClass() const
{
  return throwIfNot_(Category::CLASSTYPE), dynamic_cast<TClass *>(tDict_);
}

TEnum *
art::TypeWithDict::
tEnum() const
{
  return throwIfNot_(Category::ENUMTYPE), dynamic_cast<TEnum *>(tDict_);
}

TDataType *
art::TypeWithDict::
tDataType() const
{
  return throwIfNot_(Category::BASICTYPE), dynamic_cast<TDataType *>(tDict_);
}

TDictionary *
art::TypeWithDict::
dictFromTypeInfo_(std::type_info const & t [[gnu::unused]])
{
  // Check what we don't support.
  throwIfUnsupportedType(art::uniform_type_name(t));
  // Now try to find what we do support.
  TDictionary * result  = TClass::GetClass(t);
  if (!result) {
    result = TDataType::GetDataType(TDataType::GetType(t));
  }
  if (!result) {
    result = TEnum::GetEnum(t, TEnum::kAutoload);
  }
  if (!result) {
    if (t == typeid(void)) {
      // Corner case: per Bill T., for some reason void doesn't work from
      // typeid, but does from name.
      result = gROOT->GetType("void");
    } else {
      throwDictionaryNotFound("TypeWithDict::dictFromTypeInfo_", t);
    }
  }
  return result;
}

TDictionary *
art::TypeWithDict::
dictFromName_(std::string const & name [[gnu::unused]])
{
  TDictionary * result  = nullptr;

  static tbb::concurrent_unordered_map<std::string, TDictionary *> s_nameToDict;
  auto it = s_nameToDict.find(name);
  if (it != s_nameToDict.end()) {
    result = it->second;
  } else { // Need to make it ourselves.
    std::string uname = art::uniform_type_name(name);
    // Check for what we don't support:
    throwIfUnsupportedType(uname);
    // Now try to find what we do support.
    result = TClass::GetClass(uname.c_str());
    if (! result) {
      result = TEnum::GetEnum(uname.c_str(), TEnum::kAutoload);
    }
    if (! result) {
      result = gROOT->GetType(uname.c_str());
    }
    if (result) {
      s_nameToDict.emplace(name, result);
    }
  }

  return result;
}

art::TypeWithDict::Category
art::TypeWithDict::
categoryFromDict_(TDictionary * tDict)
{
  Category result { Category::NONE };
  if (tDict != nullptr) {
    if (dynamic_cast<TClass *>(tDict) != nullptr) {
      result = Category::CLASSTYPE;
    } else if (dynamic_cast<TDataType *>(tDict) != nullptr) {
      result = Category::BASICTYPE;
    } else if (dynamic_cast<TEnum *>(tDict) != nullptr) {
      result = Category::ENUMTYPE;
    } else {
      throw Exception(errors::LogicError)
        << "INTENRAL ERROR: TypeWithDict::categoryFromDict_ encountered a type "
        << tDict->GetName()
        << " of unknown category "
        << tDict->IsA()->GetName()
        << ".\n";
    }
  }
  return result;
}

art::TypeID
art::TypeWithDict::
typeIDFromDictAndCategory_(TDictionary *tDict, Category category)
{
  art::TypeID result;
  switch(category) {
  case Category::NONE:
  case Category::ENUMTYPE: // Can't get typeID from TEnum.
    break;
  case Category::BASICTYPE:
  {
    std::type_info const * t = nullptr;
    auto type = dynamic_cast<TDataType*>(tDict)->GetType();
    switch (type) {
    case kChar_t:
    case kchar:
      t = &typeid(char);
      break;
    case kShort_t:
      t = &typeid(short);
      break;
    case kInt_t:
      t = &typeid(int);
      break;
    case kLong_t:
      t = &typeid(long);
      break;
    case kFloat_t:
      t = &typeid(float);
      break;
    case kCharStar:
      t = &typeid(char*);
      break;
    case kDouble_t:
      t = &typeid(double);
      break;
    case kDouble32_t:
      t = &typeid(Double32_t);
      break;
    case kUChar_t:
      t = &typeid(unsigned char);
      break;
    case kUShort_t:
      t = &typeid(unsigned short);
      break;
    case kUInt_t:
    case kDataTypeAliasUnsigned_t:
      t = &typeid(unsigned int);
      break;
    case kULong_t:
      t = &typeid(unsigned long);
      break;
    case kLong64_t:
      t = &typeid(long long);
      break;
    case kULong64_t:
      t = &typeid(unsigned long long);
      break;
    case kBool_t:
      t = &typeid(bool);
      break;
    case kFloat16_t:
      t = &typeid(Float16_t);
      break;
    case kVoid_t:
      t = &typeid(void);
      break;
    case kDataTypeAliasSignedChar_t:
      t = &typeid(signed char);
      break;
    case kNoType_t:
    case kCounter: // Don't know what one is.
    case kBits: // Don't know what one is.
    case kOther_t:
    default:
      throw Exception(errors::LogicError)
        << "INTERNAL ERROR: TypeWithDict::typeIDFromDictAndCategory_ encountered unknown type "
        << type
        << " corresponding to a "
        << dynamic_cast<TDataType*>(tDict)->GetTypeName()
        << ".\n";
    }
    result = TypeID(*t);
  }
  break;
  case Category::CLASSTYPE:
    result = TypeID(*dynamic_cast<TClass *>(tDict)->GetTypeInfo());
    break;
  default:
    throwForUnexpectedCategory(category);
  }
  return result;
}

void
art::TypeWithDict::
throwIfNot_(Category category) const
{
  if (category_ != category) {
    throw Exception(errors::TypeConversion)
      << "TypeWithDict: requiested operation requires category "
      << to_string(category)
      << "; category of this type is actually "
      << to_string(category_)
      << ".\n";
  }
}

void
art::TypeWithDict::
throwIfInvalid_() const
{
  if (!(*this)) {
    throw Exception(errors::LogicError)
      << "TypeWithDict: requested operation on an invalid object\n.";
  }
}

std::string
art::
to_string(art::TypeWithDict::Category category)
{
  std::string result;
  switch (category) {
  case art::TypeWithDict::Category::NONE:
    result = "NONE";
    break;
  case art::TypeWithDict::Category::BASICTYPE:
    result = "BASICTYPE";
    break;
  case art::TypeWithDict::Category::CLASSTYPE:
    result = "CLASSTYPE";
    break;
  case art::TypeWithDict::Category::ENUMTYPE:
    result = "ENUMTYPE";
    break;
  default:
    throw art::Exception(art::errors::LogicError)
      << "INTENRAL ERROR: art::to_string(TypeWithDict::Category) cannot interpret category #"
      << static_cast<unsigned short>(category)
      << ".\n";
  }
  return result;
}
