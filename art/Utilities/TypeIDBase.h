#ifndef FWCore_Utilities_TypeIDBase_h
#define FWCore_Utilities_TypeIDBase_h

// ======================================================================
//
// TypeIDBase - Base class for classes used to compare C++ types
//
// Note:
//   This class is not meant to be used polymorphically (which is why
// there is no virtual destructor).  Instead it is used to hold a common
// methods needed by all type comparing classes.
//
// ======================================================================

#include <typeinfo>

namespace art {
  class TypeIDBase;
  bool
    operator > ( TypeIDBase const & a, TypeIDBase const & b );
  bool
    operator != ( TypeIDBase const & a, TypeIDBase const & b );
}

// ----------------------------------------------------------------------

class art::TypeIDBase
{
public:
  struct Def { };

  // c'tors:
  TypeIDBase( )
  : t_( & typeid(Def) )
  { }
  explicit TypeIDBase( std::type_info const & t)
  : t_( &t )
  { }

  // use compiler-generated copy c'tor, copy assignment, and d'tor

  // Returned C-style string owned by system; do not delete[] it.
  // This is the (horrible, mangled, platform-dependent) name of the type.
  char const *
    name( ) const
  { return t_->name(); }

  // comparators:
  bool
    operator < ( const TypeIDBase & other ) const
  { return t_->before(*other.t_); }
  bool
    operator == ( const TypeIDBase & other ) const
  { return *t_ == *other.t_; }

protected:
  std::type_info const &
    typeInfo( ) const { return *t_; }

private:
  // NOTE: since (a) the compiler generates the type_info's, and
  // (b) they have a lifetime good for the entire application,
  // we do not have to delete it.
  // We use a pointer rather than a reference so that assignment will work
  const std::type_info * t_;

};  // TypeIDBase

// ----------------------------------------------------------------------

inline bool
  art::operator > ( TypeIDBase const & a, TypeIDBase const & b )
{ return b < a; }

inline bool
  art::operator != ( TypeIDBase const & a, TypeIDBase const & b )
{ return ! (a==b); }

// ======================================================================

#endif  // FWCore_Utilities_TypeIDBase_h
