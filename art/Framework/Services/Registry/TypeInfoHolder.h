#ifndef ServiceRegistry_TypeInfoHolder_h
#define ServiceRegistry_TypeInfoHolder_h

#include <typeinfo>

//
// TypeInfoHolder - wrapper to allow std::type_info to be used as a key
//                  to a std::map
//

namespace art {
  namespace serviceregistry {

    class TypeInfoHolder
    {
      // non-assignable:
      void operator = ( TypeInfoHolder const & );

    public:
      // c'tor:
      TypeInfoHolder( std::type_info const & info )
      : info_(info)
      { }

      // accessor:
      std::type_info const &
        info( ) const
      { return info_; }

      // comparator:
      bool
        operator < ( TypeInfoHolder const & other ) const
      { return info_.before(other.info_); }

    private:
      std::type_info const &  info_;

    };  // TypeInfoHolder

  }  // serviceregistry
}  // art

#endif  // ServiceRegistry_TypeInfoHolder_h
