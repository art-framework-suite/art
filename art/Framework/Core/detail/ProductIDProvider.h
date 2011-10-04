#ifndef art_Framework_Core_detail_ProductIDProvider_h
#define art_Framework_Core_detail_ProductIDProvider_h

#include "art/Persistency/Provenance/ProductID.h"

// Two classes to provide ProductID information from a particular source
// on demand.
//
// Used by the inter-product reference system.
namespace art {
  namespace detail {
    // General per-item ProductID provider: assumes an id method for the item.
    struct ProductIDProvider;
    // Saves a constant ProductID for retrieval on demand.
    struct ConstantProductIDProvider;
  }
}

struct art::detail::ProductIDProvider {
  template <typename T>
  ProductID
  operator()(T const & t) const;
};

class art::detail::ConstantProductIDProvider {
public:
  ConstantProductIDProvider(ProductID const & pid);
  template <typename T>
  ProductID const &
  operator()(T const &) const;
  ProductID const &
  operator()() const;
private:
  ProductID const & pid_;
};

template <typename T>
inline
art::ProductID
art::detail::ProductIDProvider::
operator()(T const & t) const
{
  return t.id();
}

inline
art::detail::ConstantProductIDProvider::
ConstantProductIDProvider(ProductID const & pid)
  :
  pid_(pid)
{ }

template <typename T>
inline
art::ProductID const &
art::detail::ConstantProductIDProvider::
operator()(T const &) const
{
  return pid_;
}

inline
art::ProductID const &
art::detail::ConstantProductIDProvider::
operator()() const
{
  return pid_;
}
#endif /* art_Framework_Core_detail_ProductIDProvider_h */

// Local Variables:
// mode: c++
// End:
