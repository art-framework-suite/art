#ifndef art_Persistency_Common_detail_maybeCastObj_h
#define art_Persistency_Common_detail_maybeCastObj_h

#include <typeinfo>

namespace art {
  namespace detail {
    template <typename element_type>
    void const *
    maybeCastObj(element_type const * address,
                 std::type_info const & tiTo);
    void const *
    maybeCastObj(void const * address,
                 std::type_info const & tiFrom,
                 std::type_info const & tiTo);
  }
}

template <class element_type>
inline
void const *
art::detail::maybeCastObj(element_type const * address,
                          const std::type_info & tiTo)
{
  static std::type_info const & tiFrom(typeid(element_type));
  return maybeCastObj(address, tiFrom, tiTo);
}

#endif /* art_Persistency_Common_detail_maybeCastObj_h */

// Local Variables:
// mode: c++
// End:
