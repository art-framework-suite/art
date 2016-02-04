#ifndef art_Persistency_Common_setPtr_h
#define art_Persistency_Common_setPtr_h

#include "art/Persistency/Common/detail/maybeCastObj.h"
#include "cetlib/map_vector.h"
#include "cetlib/demangle.h"

#include <iterator>
#include <typeinfo>

namespace art {
  template <class COLLECTION>
  void
  setPtr(COLLECTION const & coll,
         const std::type_info & iToType,
         unsigned long iIndex,
         void const *& oPtr);

  template <typename T>
  void
  setPtr(cet::map_vector<T> const & obj,
         const std::type_info & iToType,
         unsigned long iIndex,
         void const *& oPtr);
}

template <class COLLECTION>
void
art::setPtr(COLLECTION const & coll,
            const std::type_info & iToType,
            unsigned long iIndex,
            void const *& oPtr)
{
  typedef COLLECTION product_type;
  auto it = coll.begin();
  advance(it, iIndex);
  oPtr = detail::maybeCastObj(detail::GetProduct<product_type>::address(it), iToType);
}

template <typename T>
void
art::setPtr(cet::map_vector<T> const & obj,
            const std::type_info & iToType,
            unsigned long iIndex,
            void const *& oPtr)
{
  detail::value_type_helper vh;
  std::string const wanted_type = cet::demangle_symbol(iToType.name());
  static size_t pos = vh.look_past_pair<T>();
  if ((pos < wanted_type.size()) && vh.starts_with_pair(wanted_type, pos)) {
    // Want value_type, not mapped_type;
    auto it = obj.find(cet::map_vector_key(iIndex));
    oPtr = detail::maybeCastObj((it == obj.end()) ? 0 : & (*it), iToType);
  }
  else {
    oPtr = detail::maybeCastObj(obj.getOrNull(cet::map_vector_key(iIndex)), iToType);
  }
}

#endif /* art_Persistency_Common_setPtr_h */

// Local Variables:
// mode: c++
// End:
