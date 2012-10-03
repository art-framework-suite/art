#ifndef art_Persistency_Common_detail_maybeCastObj_h
#define art_Persistency_Common_detail_maybeCastObj_h

#include "cetlib/demangle.h"
#include "cetlib/exception.h"

#include <typeinfo>

#include "Reflex/Object.h"
#include "Reflex/Type.h"

namespace art {
  namespace detail {
    template <typename element_type>
    void const *
    maybeCastObj(element_type const * address,
                 const std::type_info & iToType);
  }
}

template <class element_type>
void const *
art::detail::maybeCastObj(element_type const * address,
                          const std::type_info & iToType)
{
  if (iToType == typeid(element_type)) {
    return address;
  }
  else {
    using Reflex::Type;
    using Reflex::Object;
    static const Type s_type(Type::ByTypeInfo(typeid(element_type)));
    // The const_cast below is needed because
    // Object's constructor requires a pointer to
    // non-const void, although the implementation does not, of
    // course, modify the object to which the pointer points.
    Object obj(s_type, const_cast<void *>(static_cast<const void *>(address)));
    Object cast = obj.CastObject(Type::ByTypeInfo(iToType));
    if (0 != cast.Address()) {
      return cast.Address(); // returns void*, after pointer adjustment
    }
    else {
      throw cet::exception("TypeConversionError")
          << "art::Wrapper<> : unable to convert type "
          << cet::demangle_symbol(typeid(element_type).name())
          << " to "
          << cet::demangle_symbol(iToType.name())
          << "\n";
    }
  }
}


#endif /* art_Persistency_Common_detail_maybeCastObj_h */

// Local Variables:
// mode: c++
// End:
