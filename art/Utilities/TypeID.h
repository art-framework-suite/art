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

class TypeID {

public:

  TypeID();

  explicit
  TypeID(std::type_info const&);

  explicit
  TypeID(std::type_info const*);

  template<typename T>
  explicit
  TypeID(T const& val)
    : ti_(&typeid(val))
  {
  }

  // Print out the name of the type, using the reflection class name.
  void
  print(std::ostream&) const;

  // Returned C-style string owned by system; do not delete[] it.
  // This is the (horrible, mangled, platform-dependent) name of the type.
  char const*
  name() const
  {
    return ti_->name();
  }

  std::string
  className() const;

  std::string
  friendlyClassName() const;

  // Does ROOT have access to dictionary information for this type?
  bool
  hasDictionary() const;

  bool
  operator<(TypeID const& rhs) const
  {
    return ti_->before(*rhs.ti_);
  }

  bool
  operator==(TypeID const& rhs) const
  {
    return *ti_ == *rhs.ti_;
  }

  // Are we valid?
  explicit
  operator bool() const
  {
    return ti_ != &typeid(Def);
  }

  // Access the typeinfo.
  std::type_info const&
  typeInfo() const
  {
    return *ti_;
  }

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
bool
operator>(TypeID const& a, TypeID const& b)
{
  return b < a;
}

inline
bool
operator!=(TypeID const& a, TypeID const& b)
{
  return !(a == b);
}

std::ostream&
operator<<(std::ostream&, const TypeID&);

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif // art_Utilities_TypeID_h
