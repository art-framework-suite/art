#ifndef ART_PERSISTENCY_PROVENANCE_TYPEWITHDICT_H
#define ART_PERSISTENCY_PROVENANCE_TYPEWITHDICT_H
// vim: set sw=2:

#include "canvas/Utilities/TypeID.h"
#include <ostream>
#include <string>
#include <typeinfo>

class TClass;
class TDataType;
class TDictionary;
class TEnum;

namespace art {

/// \class TypeWithDict
///
/// \brief Type information using ROOT dictionaries.
///
/// Inspired by the TypeWithDict class from CMSSW, but substantially
/// simpler due to fewer requirements for type introspection in art.
///
class TypeWithDict {

public:

  /// \enum Category
  ///
  /// \brief Represent the different categories of type.
  enum class Category {
    NONE, // 0
    CLASSTYPE, // 1
    ENUMTYPE, // 2
    BASICTYPE // 3
  };

public:

  TypeWithDict()
    : tDict_(nullptr)
    , category_(Category::NONE)
    , id_()
  {
  }

  explicit
  TypeWithDict(std::type_info const& t)
    : tDict_(dictFromTypeInfo_(t))
    , category_(categoryFromDict_(tDict_))
    , id_(t)
  {
  }

  explicit
  TypeWithDict(TypeID const& id)
    : tDict_(id ? dictFromTypeInfo_(id.typeInfo()) : nullptr)
    , category_(categoryFromDict_(tDict_))
    , id_(typeIDFromDictAndCategory_(tDict_, category_))
  {
  }

  explicit
  TypeWithDict(std::string const& name)
    : tDict_(dictFromName_(name))
    , category_(categoryFromDict_(tDict_))
    , id_(typeIDFromDictAndCategory_(tDict_, category_))
  {
  }

  Category
  category() const
  {
    return category_;
  }

  /// \brief Object validity.
  explicit
  operator bool() const
  {
    return (category_ == Category::ENUMTYPE) || bool(id_);
  }

  /// \name non-ROOT information access.
  TypeID const&
  id() const
  {
    return id_;
  }

  std::type_info const&
  typeInfo() const;

  void
  print(std::ostream& os) const;

  char const*
  name() const;

  std::string
  className() const;

  std::string
  friendlyClassName() const;

  /// \name ROOT information access.
  ///
  ///{
  /// \throws Exception() if not appropriate for category.

  TClass*
  tClass() const;

  TEnum*
  tEnum() const;

  TDataType*
  tDataType() const;

  TDictionary*
  tDictionary() const
  {
    return tDict_;
  }

  ///}

private:

  /// Obtain the TDictionary for the provided type.
  static
  TDictionary*
  dictFromTypeInfo_(std::type_info const& t);

  static
  TDictionary*
  dictFromName_(std::string const& name);

  /// Obtain the category for the provided type information.
  static
  Category
  categoryFromDict_(TDictionary* tDict);

  /// Obtain the std::type_info for the provided type information and category.
  static
  TypeID
  typeIDFromDictAndCategory_(TDictionary* tDict, Category category);

private:

  TDictionary* tDict_;
  Category category_;
  TypeID id_;

};

std::string to_string(TypeWithDict::Category category);

inline
std::ostream&
operator<<(std::ostream& os, TypeWithDict::Category category)
{
  os << to_string(category);
  return os;
}

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif // ART_PERSISTENCY_PROVENANCE_TYPEWITHDICT_H
