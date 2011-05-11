#ifndef art_Persistency_Common_CollectionUtilities_h
#define art_Persistency_Common_CollectionUtilities_h
////////////////////////////////////////////////////////////////////////
// CollectionUtilities.h
//
// A collection of possibly-useful free function templates for use with
// collections. In all the below signatures, the following variables are
// used:
//
//   std::vector<COLLECTION<PROD> const *> const &in
//
//   COLLECTION<PROD> &out
//
//   OFFSET_CONTAINER<COLLECTION::size_type> &offsets; // Output.
//
//   OFFSET_CONTAINER<COLLECTION::size_type> const &offsets; // Input.
//
// OFFSET_CONTAINER and COLLECTION are arbitrary (and not necessarily
// identical) container types which satisfy the constraints as specified
// below. Most people will probably want std::vector unless they wish to
// safely and efficiently expand or shrink their list often (by adding
// and removing items, for example).
//
// OFFSET_CONTAINER must support clear(), reserve() and push_back();
//
// COLLECTION must support insert() specifying iterators, const and
// non-const begin() and end(), and reserve(). Its iterators must support
// preincrement (operator++), comparisons and dereferencing (operator*).
//
// Some functions below expect COLLECTION to be a PtrVector<PROD> or
// other collection of Ptr<PROD>. This will be specified in the
// documentation for that function.
////////////////////////////////////
//
// concatContainers(out, in)
//
// A general, simple function to append the contents of the container
// specified by the second argument to the container specified by the
// first.
//
// flattenCollections(...)
//
// A set of function templates to aid in the flattening of multiple
// collections (as described usually as a vector of const pointers to
// colletions) into one colletion, as required by acitivities such as
// product mixing. The interfaces are as follows:
//
// 1. Flatten a vector of simple collections to a single collection
// (*not* a PtrVector or other collection of Ptr -- cf 3. below or
// PtrRemapper).
//
//      flattenCollections(in, out)
//
// 2. Flatten a vector of collections to a single collection, filling a
// vector of offests into the resultant collection (eg for use by
// PtrRemapper later).
//
//
// 3. Flatten a vector of *compatible* PtrVectors (ie they all point to
// the same product) to a single PtrVector. If they require remapping,
// do *not* use this method -- use PtrRemapper instead.
//
//
// 4. Flatten a vector of *compatible* PtrVectors, filling a vector of
// offsets into the resultant collection (eg for use by a
// PtrRemapper). This function is only useful in the (hopefully rare)
// case that one has a Ptr *into* a PtrVector.
//
////////////////////////////////////////////////////////////////////////

#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/PtrVector.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "cpp0x/type_traits"
#include <cstddef>
#include <vector>

namespace art {
  // Forward declaration.
  class EDProductGetter;

  // Append container in to container out.
  template <typename CONTAINER>
  void
  concatContainers(CONTAINER &out, CONTAINER const &in);

  // 1.
  template <typename COLLECTION>
  void
  flattenCollections(std::vector<COLLECTION const *> const &in,
                     COLLECTION &out);

  // 2.
  template <typename COLLECTION, typename OFFSETS>
  void
  flattenCollections(std::vector<COLLECTION const *> const &in,
                     COLLECTION &out,
                     OFFSETS &offsets);

  // 3.
  template <typename T>
  void
  flattenCollections(std::vector<PtrVector<T> const *> const &in,
                     PtrVector<T> &out);

  // 4.
  template <typename T, typename OFFSETS>
  void
  flattenCollections(std::vector<PtrVector<T> const *> const &in,
                     PtrVector<T> &out,
                     OFFSETS &offsets);
}

////////////////////////////////////////////////////////////////////////
// No user-serviceable parts below.
////////////////////////////////////////////////////////////////////////

namespace art {
  namespace detail {
    // A. Verify a collection of PtrVector const *
    template <typename T>
    bool
    verifyPtrCollection(std::vector<art::PtrVector<T> const *> const &in);

    // B. Verify a collection (including PtrVector) of Ptrs.
    template <typename iterator>
    bool
    verifyPtrCollection(iterator beg,
                        iterator end,
                        art::ProductID id = art::ProductID(),
                        art::EDProductGetter const *getter = 0);
  }
}

// A.
template <typename T>
bool
art::detail::verifyPtrCollection(std::vector<art::PtrVector<T> const *> const &in) {
  return verifyCollection(in.begin(), in.end());
}

// B.
template <typename iterator>
bool
art::detail::verifyPtrCollection(iterator beg,
                                 iterator end,
                                 art::ProductID id = art::ProductID(),
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

template <typename CONTAINER>
void
art::concatContainers(CONTAINER &out, CONTAINER const &in) {
  out.insert(out.end(), in.begin(), in.end());
}

// 1.
template <typename COLLECTION>
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

// 2.
template <typename COLLECTION, typename OFFSETS>
void
art::flattenCollections(std::vector<COLLECTION const *> const &in,
                        COLLECTION &out,
                        OFFSETS &offsets) {
  offsets.clear();
  offsets.reserve(in.size());
  typename COLLECTION::size_type current_offset = 0;
  for (typename std::vector<COLLECTION const *>::const_iterator
         i = in.begin(),
         e = in.end();
       i != e;
       ++i) {
    typename COLLECTION::size_type current_size = (*i)->size();
    offsets.push_back(current_offset);
    current_offset += current_size;
  }
  out.reserve(current_offset);
  flattenCollections<COLLECTION>(in, out);
}

// 3.
template <typename T>
void
art::flattenCollections(std::vector<PtrVector<T> const *> const &in,
                        PtrVector<T> &out) {
  // Extra checks are required to verify that the PtrVectors are
  // compatible.
  if (!detail::verifyPtrCollection(in)) {
    // FIXME: correct this exception.
    throw Exception(errors::LogicError);
  }
  flattenCollections<PtrVector<T> >(in, out);
}

// 4.
template <typename T, typename OFFSETS>
void
art::flattenCollections(std::vector<PtrVector<T> const *> const &in,
                        PtrVector<T> &out,
                        OFFSETS &offsets) {
  // Extra checks are required to verify that the PtrVectors are
  // compatible.
  if (!detail::verifyPtrCollection(in)) {
    // FIXME: correct this exception.
    throw Exception(errors::LogicError);
  }
  flattenCollections<PtrVector<T> >(in, out, offsets);
}

#endif /* art_Persistency_Common_CollectionUtilities_h */

// Local Variables:
// mode: c++
// End:
