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
// collections (as represented usually as a vector of const pointers to
// colletions) into one collection, as required by acitivities such as
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
#include "art/Utilities/detail/metaprogramming.h"
#include "cetlib/map_vector.h"
#include "cpp0x/cstddef"
#include "cpp0x/type_traits"
#include <vector>

namespace art {
  // Forward declarations.
  class EDProductGetter;

  // Template metaprogramming.
  namespace detail {
    template <typename T, typename InIter, void (T::*)(InIter, InIter)> struct two_arg_insert_func;
    template <typename T, typename I> no_tag has_two_arg_insert_helper(...);
    template <typename T, typename I> yes_tag has_two_arg_insert_helper(two_arg_insert_func<T, I, &T::insert> *dummy);
    template <typename T>
    struct has_two_arg_insert {
      static bool const value =
        sizeof(has_two_arg_insert_helper<T, typename T::const_iterator>(0)) == sizeof(yes_tag);
    };

    template <typename T, typename RET, typename OutIter, typename InIter, RET (T::*)(OutIter, InIter, InIter)> struct three_arg_insert_func;
    template <typename T, typename R, typename O, typename I> no_tag has_three_arg_insert_helper(...);
    template <typename T, typename R, typename O, typename I> yes_tag has_three_arg_insert_helper(three_arg_insert_func<T, R, O, I, &T::insert> *dummy);
    template <typename T>
    struct has_three_arg_insert {
      static bool const value =
        sizeof(has_three_arg_insert_helper<T, void, typename T::iterator, typename T::const_iterator>(0)) == sizeof(yes_tag) ||
        sizeof(has_three_arg_insert_helper<T, typename T::iterator, typename T::const_iterator, typename T::const_iterator>(0)) == sizeof(yes_tag);
    };

    template <typename C>
    struct mix_offset {
      static
      size_t
      (C::* offset)() const;
    };

    template<>
    template <typename P>
    struct mix_offset<cet::map_vector<P> > {
      static
      size_t
      (cet::map_vector<P>::* offset)() const;
    };
  }

  template <typename C>
  size_t (C::* art::detail::mix_offset<C>::offset)() const = &C::size;

  template <typename P>
  size_t (cet::map_vector<P>::* art::detail::mix_offset<cet::map_vector<P> >::offset)() const  = &cet::map_vector<P>::delta;

  // Append container in to container out.
  // I.
  template <typename CONTAINER>
  typename std::enable_if<detail::has_two_arg_insert<CONTAINER>::value>::type
  concatContainers(CONTAINER &out, CONTAINER const &in);
  // II.
  template <typename CONTAINER>
  typename std::enable_if<detail::has_three_arg_insert<CONTAINER>::value>::type
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
  return verifyPtrCollection(in.begin(), in.end());
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
    if ((*i) != nullptr &&
        !((*i)->productGetter() && (*i)->productGetter() == getter &&
          (*i)->id().isValid() && (*i)->id() == id)) {
      return false;
    }
  }
  return true;
}

// I.
template <typename CONTAINER>
typename std::enable_if<art::detail::has_two_arg_insert<CONTAINER>::value>::type
art::concatContainers(CONTAINER &out, CONTAINER const &in) {
  (void) out.insert(in.begin(), in.end());
}
// II.
template <typename CONTAINER>
typename std::enable_if<art::detail::has_three_arg_insert<CONTAINER>::value>::type
art::concatContainers(CONTAINER &out, CONTAINER const &in) {
  out.insert(out.end(), in.begin(), in.end());
}

// 1.
template <typename COLLECTION>
void
art::flattenCollections(std::vector<COLLECTION const *> const &in,
                        COLLECTION &out) {
  typename COLLECTION::size_type total_size = 0;
  for (auto collptr : in) {
    if (collptr != nullptr) {
      total_size += collptr->size();
    }
  }
  out.reserve(total_size);
  for (auto collptr : in) {
    if (collptr != nullptr) {
      concatContainers(out, *collptr);
    }
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
  for (auto collptr : in ) {
    if (collptr != nullptr) {
      typename COLLECTION::size_type delta =
        (collptr->*detail::mix_offset<COLLECTION>::offset)();
      offsets.push_back(current_offset);
      current_offset += delta;
    }
  }
  flattenCollections<COLLECTION>(in, out); // 1.
}

// 3.
template <typename T>
void
art::flattenCollections(std::vector<PtrVector<T> const *> const &in,
                        PtrVector<T> &out) {
  // Extra checks are required to verify that the PtrVectors are
  // compatible.
  if (!detail::verifyPtrCollection(in)) {
    throw Exception(errors::LogicError)
      << "Attempt to flatten incompatible PtrVectors "
      << "referring to different ProductIDs.\n";
  }
  flattenCollections<PtrVector<T> >(in, out); // 1
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
    throw Exception(errors::LogicError)
      << "Attempt to flatten incompatible PtrVectors "
      << "referring to different ProductIDs.\n";
  }
  flattenCollections<PtrVector<T> >(in, out, offsets); // 2.
}

#endif /* art_Persistency_Common_CollectionUtilities_h */

// Local Variables:
// mode: c++
// End:
