#ifndef JJJSHDJDHJJJD
#define JJJSHDJDHJJJD

#include "art/Utilities/TypeID.h"

#include <ostream>
#include <string>
#include <typeinfo>

namespace art {
  class TypeWithDict;
}

// Forward declarations.
class TClass;
class TDataType;
class TDictionary;
class TEnum;

/// \class art::TypeWithDict
///
/// \brief Type information using ROOT dictionaries.
///
/// Inspired by the TypeWithDict class from CMSSW, but substantially
/// simpler due to fewer requirements for type introspection in art.
class art::TypeWithDict {
public:
  /// \enum Category
  ///
  /// \brief Represent the different categories of type.
  enum class Category { NONE, CLASSTYPE, ENUMTYPE, BASICTYPE };

  TypeWithDict();
  explicit TypeWithDict(std::type_info const & t);
  explicit TypeWithDict(TypeID const & id);
  explicit TypeWithDict(std::string const & name);

  Category category() const;
  /// \brief Object validity.
  explicit operator bool() const;

  /// \name non-ROOT information access.
  TypeID const & id() const;
  std::type_info const & typeInfo() const;
  void print(std::ostream & os) const;
  char const * name() const;
  std::string className() const;
  std::string friendlyClassName() const;

  /// \name ROOT information access.
  ///
  ///{
  /// \throws art::Exception() if not appropriate for category.
  TClass * tClass() const;
  TEnum * tEnum() const;
  TDataType * tDataType() const;
  TDictionary * tDictionary() const;
  ///}

private:
  /// Obtain the TDictionary for the provided type.
  static TDictionary * dictFromTypeInfo_(std::type_info const & t);
  static TDictionary * dictFromName_(std::string const & name);

  /// Obtain the category for the provided type information.
  static Category categoryFromDict_(TDictionary * tDict);
  /// Obtain the std::type_info for the provided type information and category.
  static art::TypeID
  typeIDFromDictAndCategory_(TDictionary *tDict, Category category);
  void throwIfNot_(Category category) const;
  void throwIfInvalid_() const;

  TDictionary * tDict_;
  Category category_;
  TypeID id_;
};

namespace art {
  std::string to_string(TypeWithDict::Category category);

  std::ostream & operator << (std::ostream & os,
                              TypeWithDict::Category category);
}


inline
art::TypeWithDict::
TypeWithDict()
  :
  tDict_(nullptr),
  category_(Category::NONE),
  id_()
{
}

inline
art::TypeWithDict::
TypeWithDict(std::type_info const & t)
:
  tDict_(dictFromTypeInfo_(t)),
  category_(categoryFromDict_(tDict_)),
  id_(t)
{
}

inline
art::TypeWithDict::
TypeWithDict(TypeID const & id)
:
  tDict_(id ? dictFromTypeInfo_(id.typeInfo()) : nullptr),
  category_(categoryFromDict_(tDict_)),
  // dictFromTypeInfo_ might invoke dictFromName_.
  id_(typeIDFromDictAndCategory_(tDict_, category_))
{
}

inline
art::TypeWithDict::
TypeWithDict(std::string const & name)
:
  tDict_(dictFromName_(name)),
  category_(categoryFromDict_(tDict_)),
  id_(typeIDFromDictAndCategory_(tDict_, category_))
{
}

inline
art::TypeWithDict::Category
art::TypeWithDict::
category() const
{
  return category_;
}

inline
art::TypeWithDict::
operator bool() const
{
  return (category_ == Category::ENUMTYPE) ||
    bool(id_);
}

inline
art::TypeID const &
art::TypeWithDict::
id() const
{
  return id_;
}

inline
std::type_info const &
art::TypeWithDict::
typeInfo() const {
  return throwIfInvalid_(), id_.typeInfo();
}

inline
TDictionary *
art::TypeWithDict::
tDictionary() const
{
  return tDict_;
}

inline
std::ostream &
art::operator << (std::ostream & os, art::TypeWithDict::Category category)
{
  os << to_string(category);
  return os;
}

#endif

// Local Variables:
// mode: c++
// End:
