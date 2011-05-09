#ifndef art_Framework_Core_PtrRemapper_h
#define art_Framework_Core_PtrRemapper_h

#include "art/Persistency/Common/CollectionUtilities.h"
#include "art/Persistency/Common/EDProductGetter.h"
#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/PtrVector.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "cetlib/exempt_ptr.h"
#include "cpp0x/functional"

#include <map>

namespace art {
  class PtrRemapper;

  // Utility function object to use as a default argument to 4.
  namespace detail {
    template <typename InIter, typename PROD>
    class SimpleExtractor :
      public std::unary_function<PROD const *, std::pair<InIter, InIter> > {
    public:
      typename SimpleExtractor::result_type
      operator()(typename SimpleExtractor::argument_type prod) const;
    };
  }
}

template <typename InIter, typename PROD>
typename art::detail::SimpleExtractor<InIter, PROD>::result_type
art::detail::SimpleExtractor<InIter, PROD>::operator()
  (typename SimpleExtractor<InIter, PROD>::argument_type prod) const {
  return std::make_pair(prod.begin(), prod.end());
}

class art::PtrRemapper {
public:
  typedef std::map<ProductID, ProductID> ProdTransMap_t;

  PtrRemapper();

  // Prepare the pointer remapper for use by detail objects.
  void setProdTransMap(cet::exempt_ptr<ProdTransMap_t const> prodTransMap);
  void setProductGetter(cet::exempt_ptr<EDProductGetter const> productGetter);

  // 1. Remap a single Ptr.
  template <typename PROD>
  Ptr<PROD> operator()(Ptr<PROD> const &oldPtr, size_t offset) const;

  // 2. Remap a single PtrVector.
  template <typename PROD>
  PtrVector<PROD> operator()(PtrVector<PROD> const &old, size_t offset) const;

  // 3. Remap a compatible collection (including PtrVector) of Ptr. Will
  // also remap a compatible collection of PtrVector, but not of
  // PtrVector const * -- for the latter, see 4. and 5.
  template <typename InIter, typename OutIter>
  void operator()(InIter beg,
                  InIter end,
                  OutIter dest,
                  size_t offset) const;

  // 4. Remap and flatten a set of containers of Ptrs (including
  // PtrVector) which may be a component of the provided product. If it
  // is the product itself, allow the final argument to default.
  template <typename InIter, typename OutIter, typename PROD>
  void
  operator()(std::vector<PROD const *> const &in,
             OutIter out,
             std::vector<size_t> const &offsets,
             std::function<std::pair<InIter, InIter> (PROD const *)>
             extractor) const; // = detail::SimpleExtractor<PROD, typename std::vector<PROD const *>::const_iterator>()) const;

  // 5. Remap and flatten a set of containers of Ptrs (including
  // PtrVector) which is a component of the provided product using the
  // provided accessor member function.
  template <typename PROD, typename InIter, typename OutIter, typename X>
  void
  operator()(std::vector<PROD const *> const &in,
             OutIter out,
             std::vector<size_t> const &offsets,
             std::pair<InIter, InIter> (X::*extractor) (PROD const *),
             X *x) const;

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

// 1.
template <typename PROD>
art::Ptr<PROD>
art::PtrRemapper::
operator()(Ptr<PROD> const &oldPtr,
           size_t offset) const {
  ProdTransMap_t::const_iterator iter = prodTransMap_->find(oldPtr.id());
  if (iter == prodTransMap_->end()) {
    // FIXME: correct this exception.
    throw Exception(errors::LogicError);
  }
  return Ptr<PROD>(iter->second,
                   oldPtr.key() + offset,
                   productGetter_);
}

// 2.
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

// 3.
template <typename InIter, typename OutIter>
void
art::PtrRemapper::
operator()(InIter beg,
           InIter end,
           OutIter dest,
           size_t offset) const {
  if (!detail::verifyPtrCollection(beg, end)) {
    // FIXME: correct this exception.
    throw Exception(errors::LogicError);
  }
  for (InIter i = beg;
       i != end;
       ++i) {
    *dest++ = this->operator()(*i, offset);
  }
}

// 4. 
template <typename InIter, typename OutIter, typename PROD>
void
art::PtrRemapper::
operator()(std::vector<PROD const *> const &in,
           OutIter out,
           std::vector<size_t> const &offsets,
           std::function<std::pair<InIter, InIter> (PROD const *)>
           extractor) const {
  typename std::vector<PROD const *>::const_iterator
    i = in.begin(),
    e = in.end();
  std::vector<size_t>::const_iterator
    off_iter = offsets.begin(),
    off_end = offsets.end();
  for (; i != e; ++i, ++off_iter) {
    if (off_iter == off_end) {
      throw Exception(errors::LogicError)
        << ".\n";
    }
    std::pair<InIter, InIter> i_p(extractor(*i));
    this->operator()(i_p.first, i_p.second, out, *off_iter);
  }
}


// 5.
template <typename PROD, typename InIter, typename OutIter, typename X>
void
art::PtrRemapper::
operator()(std::vector<PROD const *> const &in,
           OutIter out,
           std::vector<size_t> const &offsets,
           std::pair<InIter, InIter> (X::*extractor) (PROD const *),
           X *x) const {
  using std::placeholders::_1;
  this->operator()(in,
                   out,
                   offsets,
                   std::bind(static_cast<std::pair<InIter, InIter> (*)(PROD const *)>(extractor), x, _1));
}
#endif /* art_Framework_Core_PtrRemapper_h */

// Local Variables:
// mode: c++
// End:
