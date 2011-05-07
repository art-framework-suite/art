#ifndef art_Framework_Core_PtrRemapper_h
#define art_Framework_Core_PtrRemapper_h

#include "art/Persistency/Common/CollectionUtilities.h"
#include "art/Persistency/Common/EDProductGetter.h"
#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/PtrVector.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "cetlib/exempt_ptr.h"

#include <map>

namespace art {
  class PtrRemapper;
}

class art::PtrRemapper {
public:
  typedef std::map<ProductID, ProductID> ProdTransMap_t;

  PtrRemapper();

  // Prepare the pointer remapper for use by detail objects.
  void setProdTransMap(cet::exempt_ptr<ProdTransMap_t const> prodTransMap);
  void setProductGetter(cet::exempt_ptr<EDProductGetter const> productGetter);

  //  Remap a single Ptr.
  template <typename PROD>
  Ptr<PROD> operator()(Ptr<PROD> const &oldPtr, size_t offset) const;

  // Remap a single PtrVector.
  template <typename PROD>
  PtrVector<PROD> operator()(PtrVector<PROD> const &old, size_t offset) const;

  // Remap a compatible collection of Ptr (or of PtrVector).
  template <typename PROD, typename iterator, typename OutIter>
  void operator()(iterator beg,
                  iterator end,
                  OutIter dest,
                  size_t offset) const;

private:
  cet::exempt_ptr<ProdTransMap_t const> prodTransMap_;
  cet::exempt_ptr<EDProductGetter const> productGetter_;
};

inline
art::PtrRemapper::
PtrRemapper() : prodTransMap_() {}

inline
void
art::PtrRemapper::
setProdTransMap(cet::exempt_ptr<ProdTransMap_t const> prodTransMap) {
  prodTransMap_ = prodTransMap;
}

inline
void
art::PtrRemapper::
setProductGetter(cet::exempt_ptr<EDProductGetter const> productGetter) {
  productGetter_ = productGetter;
}

template <typename PROD>
art::Ptr<PROD>
art::PtrRemapper::
operator()(Ptr<PROD> const &oldPtr,
           size_t offset) const {
  return Ptr<PROD>(prodTransMap_[oldPtr.id()],
                   oldPtr.key() + offset,
                   productGetter_);
}

template <typename PROD>
art::PtrVector<PROD>
art::PtrRemapper::
operator()(PtrVector<PROD> const &old,
           size_t offset) const {
  PtrVector<PROD> result;
  result.reserve(old.size());
  std::transform(old.begin(),
                 old.end(),
                 std::back_inserter(result),
                 std::bind(static_cast<Ptr<PROD> (PtrRemapper::*)(Ptr<PROD> const &, size_t)>
                           (&PtrRemapper::operator()), this, _1, offset));
  return result;
}

template <typename PROD, typename iterator, typename OutIter>
void
art::PtrRemapper::
operator()(iterator beg,
           iterator end,
           OutIter dest,
           size_t offset) const {
  detail::verifyPtrCollection(beg, end);
  for (iterator i = beg;
       i != end;
       ++i) {
    *dest++ = this->operator()(*i, offset);
  }
}

#endif /* art_Framework_Core_PtrRemapper_h */

// Local Variables:
// mode: c++
// End:
