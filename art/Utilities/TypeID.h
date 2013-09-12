#ifndef art_Utilities_TypeID_h
#define art_Utilities_TypeID_h

/*----------------------------------------------------------------------

TypeID: A unique identifier for a C++ type.

The identifier is unique within an entire program, but can not be
persisted across invocations of the program.

----------------------------------------------------------------------*/
#include "art/Utilities/fwd.h"

#include <iosfwd>
#include <typeinfo>
#include <string>

namespace art {

  bool operator > ( TypeID const & a, TypeID const & b );
  bool operator != ( TypeID const & a, TypeID const & b );

  std::ostream& operator<<(std::ostream& os, const TypeID& id);
}

class art::TypeID {
public:

  TypeID() : t_(&typeid(Def)) {}

  explicit TypeID(std::type_info const &t) : t_(&t) {}

  template <typename T>
  explicit TypeID(const T& t) : t_(&typeid(t)) {}

  // Print out the name of the type, using the reflection class name.
  void print(std::ostream& os) const;

  // Returned C-style string owned by system; do not delete[] it.
  // This is the (horrible, mangled, platform-dependent) name of the type.
  char const * name() const { return t_->name(); }

  std::string persistentClassName() const;

  std::string stdClassName() const;

  std::string friendlyClassName() const;

  bool hasDictionary() const;

  // comparators:
  bool
  operator < ( const TypeID & other ) const
  { return t_->before(*other.t_); }
  bool
  operator == ( const TypeID & other ) const
  { return *t_ == *other.t_; }

  operator bool() const { return t_ != &typeid(Def); }

  std::type_info const &typeInfo() const { return *t_; }

private:
  struct Def {};

  static bool stripTemplate(std::string& theName);

  static bool stripNamespace(std::string& theName);

  // NOTE: since (a) the compiler generates the type_infos, and
  // (b) they have a lifetime good for the entire application,
  // we do not have to delete it.
  // We use a pointer rather than a reference so that assignment will work
  const std::type_info * t_;
};

inline bool
  art::operator > ( TypeID const & a, TypeID const & b )
{ return b < a; }

inline bool
  art::operator != ( TypeID const & a, TypeID const & b )
{ return ! (a==b); }

#endif /* art_Utilities_TypeID_h */

// Local Variables:
// mode: c++
// End:
