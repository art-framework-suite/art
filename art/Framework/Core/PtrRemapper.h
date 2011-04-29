#ifndef art_Framework_Core_PtrRemapper_h
#define art_Framework_Core_PtrRemapper_h

#include "art/Persistency/Common/EDProductGetter.h"
#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Provenance/BranchID.h"

#include <map>

namespace art {
  class PtrRemapper;
}

class art::PtrRemapper {
public:
  typedef std::map<BranchID, BranchID> ProdTransMap_t;

  PtrRemapper();
  explicit PtrRemapper(ProdTransMap_t const &prodTransMap);

  void setProdTransMap(ProdTransMap_t const &prodTransMap);
  void setProductGetter(EDProductGetter const *productGetter);

  template <typename PROD>
  Ptr<PROD> operator()(Ptr<PROD> const &oldPtr,
                       size_t offset) const;
private:
  ProdTransMap_t prodTransMap_;
  EDProductGetter const *productGetter_;
};

inline
art::PtrRemapper::
PtrRemapper() : prodTransMap_() {}

inline
art::PtrRemapper::
PtrRemapper(ProdTransMap_t const &prodTransMap)
  :
  prodTransMap_(prodTransMap)
{}

inline
void
art::PtrRemapper::
setProdTransMap(ProdTransMap_t const &prodTransMap) {
  prodTransMap_ = prodTransMap;
}

inline
void
art::PtrRemapper::
setProductGetter(EDProductGetter const *productGetter) {
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
