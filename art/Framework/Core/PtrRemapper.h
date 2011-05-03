#ifndef art_Framework_Core_PtrRemapper_h
#define art_Framework_Core_PtrRemapper_h

#include "art/Persistency/Common/EDProductGetter.h"
#include "art/Persistency/Common/Ptr.h"
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
  explicit PtrRemapper(cet::exempt_ptr<ProdTransMap_t const> prodTransMap);

  void setProdTransMap(cet::exempt_ptr<ProdTransMap_t const> prodTransMap);
  void setProductGetter(cet::exempt_ptr<EDProductGetter const> productGetter);

  template <typename PROD>
  Ptr<PROD> operator()(Ptr<PROD> const &oldPtr,
                       size_t offset) const;
private:
  cet::exempt_ptr<ProdTransMap_t const> prodTransMap_;
  cet::exempt_ptr<EDProductGetter const> productGetter_;
};

inline
art::PtrRemapper::
PtrRemapper() : prodTransMap_() {}

inline
art::PtrRemapper::
PtrRemapper(cet::exempt_ptr<ProdTransMap_t const> prodTransMap)
  :
  prodTransMap_(prodTransMap)
{}

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
#endif /* art_Framework_Core_PtrRemapper_h */

// Local Variables:
// mode: c++
// End:
