#ifndef art_Persistency_Common_getElementAddresses_h
#define art_Persistency_Common_getElementAddresses_h

#include "canvas/Persistency/Common/detail/maybeCastObj.h"

#include <string>
#include <typeinfo>
#include <vector>

namespace art {
  template <class COLLECTION>
  void
  getElementAddresses(COLLECTION const & coll,
                      const std::type_info & iToType,
                      const std::vector<unsigned long>& iIndices,
                      std::vector<void const *>& oPtr);

  template <typename T>
  void
  getElementAddresses(cet::map_vector<T> const & obj,
                      const std::type_info & iToType,
                      const std::vector<unsigned long>& iIndices,
                      std::vector<void const *>& oPtr);

  namespace detail {
    class value_type_helper;
  }
}

class art::detail::value_type_helper {
public:
  static std::string const & pair_stem() {
    static std::string const pair_stem_s("std::pair<");
    return pair_stem_s;
  }

  static size_t pair_stem_offset() {
    static size_t const pair_stem_offset_s = pair_stem().size();
    return pair_stem_offset_s;
  }

  bool starts_with_pair(std::string const & type_name, size_t pos = 0) {
    return (type_name.compare(pos, pair_stem_offset(), pair_stem()) == 0);
  }

  template <typename T>
  size_t look_past_pair() {
    static std::string const mapped_type = cet::demangle_symbol(typeid(T).name());
    size_t pos = 0;
    while (starts_with_pair(mapped_type, pos)) { pos += pair_stem_offset(); }
    return pos;
  }
};

template <class COLLECTION>
void
art::getElementAddresses(COLLECTION const & coll,
                         const std::type_info & iToType,
                         const std::vector<unsigned long>& iIndices,
                         std::vector<void const *>& oPtr)
{
  typedef COLLECTION product_type;
  oPtr.reserve(iIndices.size());
  for (std::vector<unsigned long>::const_iterator
       itIndex = iIndices.begin(),
       itEnd = iIndices.end();
       itIndex != itEnd;
       ++itIndex) {
    auto it = coll.begin();
    advance(it, *itIndex);
    oPtr.push_back(detail::maybeCastObj(detail::GetProduct<product_type>::address(it), iToType));
  }
}

template <typename T>
void
art::getElementAddresses(cet::map_vector<T> const & obj,
                         const std::type_info & iToType,
                         const std::vector<unsigned long>& iIndices,
                         std::vector<void const *>& oPtr)
{
  typedef cet::map_vector<T> product_type;
  typedef typename product_type::const_iterator iter;
  detail::value_type_helper vh;
  std::string const wanted_type = cet::demangle_symbol(iToType.name());
  static size_t pos = vh.look_past_pair<T>();
  oPtr.reserve(iIndices.size());
  if ((pos < wanted_type.size()) &&
      vh.starts_with_pair(wanted_type, pos)) {
    // Want value_type.
    for (std::vector<unsigned long>::const_iterator
         itIndex = iIndices.begin(),
         itEnd = iIndices.end();
         itIndex != itEnd;
         ++itIndex) {
      iter it = obj.find(cet::map_vector_key(*itIndex));
      oPtr.push_back(detail::maybeCastObj((it == obj.end()) ? 0 : & (*it),
                                          iToType));
    }
  }
  else {
    // Want mapped_type.
    for (std::vector<unsigned long>::const_iterator
         itIndex = iIndices.begin(),
         itEnd = iIndices.end();
         itIndex != itEnd;
         ++itIndex) {
      oPtr.push_back(detail::maybeCastObj
                     (obj.getOrNull(cet::map_vector_key(*itIndex)),
                      iToType));
    }
  }
}

#endif /* art_Persistency_Common_getElementAddresses_h */

// Local Variables:
// mode: c++
// End:
