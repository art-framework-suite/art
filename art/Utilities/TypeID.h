#ifndef art_Utilities_TypeID_h
#define art_Utilities_TypeID_h

////////////////////////////////////////////////////////////////////////
// TypeID: A unique identifier for a C++ type.
//
// The identifier is unique within an entire program, but cannot be
// persisted across invocations of the program.
////////////////////////////////////////////////////////////////////////
#include "art/Utilities/fwd.h"

#include <iosfwd>
#include <string>
#include <typeinfo>

namespace art {

  bool operator > ( TypeID const & a, TypeID const & b );
  bool operator != ( TypeID const & a, TypeID const & b );

  std::ostream& operator<<(std::ostream& os, const TypeID& id);
}

class art::TypeID {
public:

  // Constructors.
  TypeID() : t_(&typeid(Def)) {}
  explicit TypeID(std::type_info const &t);
  template <typename T> explicit TypeID(const T& t);

  // Print out the name of the type, using the reflection class name.
  void print(std::ostream& os) const;

  // Returned C-style string owned by system; do not delete[] it.
  // This is the (horrible, mangled, platform-dependent) name of the type.
  char const * name() const;
  std::string className() const;
  std::string friendlyClassName() const;

  // Does ROOT have access to dictionary information for this type?
  bool hasDictionary() const;

  // Comparators:
  bool operator < ( const TypeID & other ) const;
  bool operator == ( const TypeID & other ) const;

  // Are we valid?
  explicit operator bool() const;

  // Access the typeinfo.
  std::type_info const &typeInfo() const;

private:
  struct Def {};

  // NOTE: since (a) the compiler generates the type_infos, and
  // (b) they have a lifetime good for the entire application,
  // we do not have to delete it.
  // We use a pointer rather than a reference so that assignment will work
  const std::type_info * t_;
};

inline
bool
art::operator > ( TypeID const & a, TypeID const & b )
{
  return b < a;
}

inline
bool
art::operator != ( TypeID const & a, TypeID const & b )
{
  return ! (a==b);
}

inline
art::TypeID::TypeID(std::type_info const & t)
:
  t_(&t)
{
}

template <typename T>
inline
art::TypeID::TypeID(T const & t)
:
  t_(&typeid(t))
{
}

inline
char const *
art::TypeID::name() const
{
  return t_->name();
}

inline
bool
art::TypeID::operator < (TypeID const & other) const
{
  return t_->before(*other.t_);
}

inline
bool
art::TypeID::operator == (TypeID const & other) const
{
  return *t_ == *other.t_;
}

inline
art::TypeID::operator bool() const
{
  return t_ != &typeid(Def);
}

inline
std::type_info const &
art::TypeID::typeInfo() const
{
  return *t_;
}

#endif /* art_Utilities_TypeID_h */

// Local Variables:
// mode: c++
// End:
