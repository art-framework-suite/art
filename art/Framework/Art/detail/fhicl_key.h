#ifndef art_Framework_Art_detail_fhicl_key_h
#define art_Framework_Art_detail_fhicl_key_h

// Class for concatenating FHiCL names into a dot-delimited FHiCL key

#include <string>

namespace art::detail {

  template <typename T>
  std::enable_if_t<std::is_convertible_v<T, std::string>, std::string>
  fhicl_key(T const& name)
  {
    return name;
  }

  template <typename H, typename... T>
  std::enable_if_t<std::is_convertible_v<H, std::string>, std::string>
  fhicl_key(H const& hname, T const&... tnames)
  {
    std::string const head{hname};
    return head.empty() ? fhicl_key(tnames...) :
                          head + "." + fhicl_key(tnames...);
  }
} // namespace art::detail

#endif /* art_Framework_Art_detail_fhicl_key_h */

// Local variables:
// mode: c++
// End:
