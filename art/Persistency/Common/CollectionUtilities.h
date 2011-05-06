#ifndef art_Persistency_Common_CollectionUtilities_h
#define art_Persistency_Common_CollectionUtilities_h

#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/PtrVector.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "cpp0x/type_traits"
#include <cstddef>
#include <vector>

namespace art {
  // Forward declaration
  class EDProductGetter;

  // Append container in to container out.
  template <class CONTAINER>
  void
  concatContainers(CONTAINER &out, CONTAINER const &in);

  // Flatten a vector of collections to a single collection.
  template <class COLLECTION>
  void
  flattenCollections(std::vector<COLLECTION const *> const &in,
                     COLLECTION &out);

  // Flatten a vector of compatible PtrVectors to a single PtrVector.
  template <typename T>
  void
  flattenCollections(std::vector<PtrVector<T> const *> const &in,
                     PtrVector<T> &out);

  // Flatten a vector of collections to a single collection, filling a
  // vector of offests into the resultant collection (eg for use by
  // PtrRemapper later).
  template <class COLLECTION>
  void
  flattenCollections(std::vector<COLLECTION const *> const &in,
                     COLLECTION &out,
                     std::vector<size_t> &offsets);

  // Flatten a vector of compatible PtrVectors, filling a vector of
  // offsets into the resultant collection (eg for use by a
  // PtrRemapper). This function is only useful in the (hopefully rare)
  // case that one has a Ptr *into* a PtrVector.
  template <typename T>
  void
  flattenCollections(std::vector<PtrVector<T> const *> const &in,
                     PtrVector<T> &out,
                     std::vector<size_t> &offsets);
}

////////////////////////////////////////////////////////////////////////
// No user-servicable parts below.
////////////////////////////////////////////////////////////////////////

namespace art {
  namespace detail {
    // Verify a collection of PtrVector const *
    template <typename T>
    void
    verifyPtrCollection(std::vector<art::PtrVector<T> const *> const &in);

    // Verify a collection of PtrVector const * (alternative interface).
    template <typename iterator>
    typename std::enable_if<std::is_base_of<art::PtrVectorBase, typename iterator::value_type>::value>::type
    verifyPtrCollection(iterator beg, iterator end);

    // Verify a collection of Ptrs.
    template <typename iterator>
    bool
    verifyPtrCollection(iterator beg,
                        iterator end,
                        art::ProductID const &id = art::ProductID(),
                        art::EDProductGetter const *getter = 0);

    // Verify a collection of const pointers to collections of Ptrs.
    template <typename iterator>
    typename std::enable_if<!std::is_base_of<art::PtrVectorBase, typename iterator::value_type>::value>::type
    verifyPtrCollection(iterator beg, iterator end);
  }
}

template <typename T>
void
art::detail::verifyPtrCollection(std::vector<art::PtrVector<T> const *> const &in) {
  verifyCollection(in.begin(), in.end());
}

template <typename iterator>
typename std::enable_if<std::is_base_of<art::PtrVectorBase, typename iterator::value_type>::value>::type
art::detail::verifyPtrCollection(iterator beg, iterator end) {
  if (beg == end) return;
  art::ProductID id(beg->id());
  art::EDProductGetter *getter(beg->productGetter());
  for (iterator i = beg;
       i != end;
       ++i) {
    if (!((*i)->productGetter() && (*i)->productGetter() == getter &&
          (*i)->id().isValid() && (*i)->id() == id)) {
      throw art::Exception(art::errors::LogicError)
        << "Cannot concatenate this set of PtrVectors because they "
        << "do not refer to the same collection in the same event.\n"
        << "Should you have remapped them first?\n";
    }
  }
}

template <typename iterator>
bool
art::detail::verifyPtrCollection(iterator beg,
                                 iterator end,
                                 art::ProductID const &id = art::ProductID(),
                                 art::EDProductGetter const *getter = 0) {
  if (beg == end) return true;
  if (!id.isValid()) {
    id = (*beg).id();
  }
  if (!getter) {
    getter = (*beg).productGetter();
  }
  for (iterator i = beg;
       i != end;
       ++i) {
    if (!((*i).productGetter() && (*i).productGetter() == getter &&
          (*i).id().isValid() && (*i).id() == id)) {
      return false;
    }
  }
  return true;
}

template <typename iterator>
typename std::enable_if<!std::is_base_of<art::PtrVectorBase, typename iterator::value_type>::value>::type
art::detail::verifyPtrCollection(iterator beg, iterator end) {
  if (beg == end) return true;
  art::ProductID id;
  art::EDProductGetter *getter = 0;
  for (iterator i = beg;
       i != end;
       ++i) {
    if (!(getter || (*i)->empty())) {
      id = (*i)->front().id();
      getter = (*i)->front.productGetter();
    }
    if (!verifyPtrCollection(*i, id, getter)) {
      throw art::Exception(art::errors::LogicError)
        << "Cannot concatenate this set of containers of Ptr because they "
        << "do not refer to the same collection in the same event.\n"
        << "Should you have remapped them first?\n";
    }
  }
}

template <class CONTAINER>
void
art::concatContainers(CONTAINER &out, CONTAINER const &in) {
  out.insert(out.end(), in.begin(), in.end());
}

template <class COLLECTION>
void
art::flattenCollections(std::vector<COLLECTION const *> const &in,
                        COLLECTION &out) {
  for(typename std::vector<COLLECTION const *>::const_iterator
        i = in.begin(),
        e = in.end();
      i != e;
      ++i) {
    concatContainers(out, **i);
  }
}

template <typename T>
void
art::flattenCollections(std::vector<PtrVector<T> const *> const &in,
                        PtrVector<T> &out) {
  // Extra checks are required to verify that the PtrVectors are
  // compatible.
  detail::verifyPtrCollection(in);
  flattenCollections<PtrVector<T> >(in, out);
}

template <class COLLECTION>
void
art::flattenCollections(std::vector<COLLECTION const *> const &in,
                        COLLECTION &out,
                        std::vector<size_t> &offsets) {
  offsets.clear();
  offsets.reserve(in.size());
  size_t current_offset = 0;
  for (typename std::vector<COLLECTION const *>::const_iterator
         i = in.begin(),
         e = in.end();
       i != e;
       ++i) {
    size_t current_size = (*i)->size();
    offsets.push_back(current_offset);
    current_offset += current_size;
  }
  out.reserve(current_offset);
  flattenCollections<COLLECTION>(in, out);
}

template <typename T>
void
art::flattenCollections(std::vector<PtrVector<T> const *> const &in,
                        PtrVector<T> &out,
                        std::vector<size_t> &offsets) {
  // Extra checks are required to verify that the PtrVectors are
  // compatible.
  detail::verifyPtrCollection(in);
  flattenCollections<PtrVector<T> >(in, out, offsets);
}

#endif /* art_Persistency_Common_CollectionUtilities_h */

// Local Variables:
// mode: c++
// End:
