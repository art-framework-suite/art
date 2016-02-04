#ifndef art_Utilities_TypeID_h
#define art_Utilities_TypeID_h
// vim: set sw=2:

//
// TypeID: A unique identifier for a C++ type.
//
// The identifier is unique within an entire program, but cannot be
// persisted across invocations of the program.
//

#include "art/Utilities/fwd.h"
#include <iosfwd>
#include <string>
#include <typeinfo>

namespace art {
  class TypeID;

  bool operator > (TypeID const &, TypeID const &);
  bool operator != (TypeID const &, TypeID const &);
  std::ostream & operator << (std::ostream &, TypeID const &);
}

class art::TypeID {
public:
  TypeID();

  explicit
  TypeID(std::type_info const &);

  explicit
  TypeID(std::type_info const *);

  template<typename T>
  explicit
  TypeID(T const & val);

  // Print out the name of the type, using the reflection class name.
  void print(std::ostream &) const;

  // Returned C-style string owned by system; do not delete[] it.
  // This is the (horrible, mangled, platform-dependent) name of the type.
  char const * name() const;

  std::string className() const;

  std::string friendlyClassName() const;

  // Does ROOT have access to dictionary information for this type?
  bool hasDictionary() const;

  bool operator < (TypeID const & rhs) const;

  bool operator==(TypeID const& rhs) const;

  // Are we valid?
  explicit
  operator bool() const;

  // Access the typeinfo.
  std::type_info const & typeInfo() const;

private:

  struct Def {};

private:

  // NOTE: since (a) the compiler generates the type_infos, and
  // (b) they have a lifetime good for the entire application,
  // we do not have to delete it.
  // We use a pointer rather than a reference so that assignment will work
  std::type_info const* ti_;

};

inline
art::TypeID::TypeID()
  : ti_(&typeid(Def))
{
}

inline
art::TypeID::TypeID(std::type_info const & ti)
  : ti_(&ti)
{
}

inline
art::TypeID::TypeID(std::type_info const * ti)
  : ti_(ti)
{
}

template <typename T>
inline
art::TypeID::TypeID(T const& val)
    : ti_(&typeid(val))
{
}

inline
char const *
art::TypeID::name() const
{
  return ti_->name();
}

inline
bool
art::TypeID::operator < (TypeID const & rhs) const
{
  return ti_->before(*rhs.ti_);
}

inline
bool
art::TypeID::operator == (TypeID const & rhs) const
{
  return *ti_ == *rhs.ti_;
}

inline
art::TypeID::operator bool() const
{
  return ti_ != &typeid(Def);
}

inline
std::type_info const &
art::TypeID::typeInfo() const
{
  return *ti_;
}

inline
bool
art::operator > (TypeID const & a, TypeID const & b)
{
  return b < a;
}

inline
bool
art::operator != (TypeID const & a, TypeID const & b)
{
  return !(a == b);
}

// Local Variables:
// mode: c++
// End:
#endif // art_Utilities_TypeID_h
