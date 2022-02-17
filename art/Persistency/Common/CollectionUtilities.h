#ifndef art_Persistency_Common_CollectionUtilities_h
#define art_Persistency_Common_CollectionUtilities_h
////////////////////////////////////////////////////////////////////////
// CollectionUtilities.h
//
// A collection of possibly-useful free function templates for use with
// collections. In all the below signatures, the following variables are
// used:
//
//   std::vector<COLLECTION<PROD> const*> const& in
//
//   COLLECTION<PROD>& out
//
//   OFFSET_CONTAINER<COLLECTION::size_type>& offsets; // Output.
//
//   OFFSET_CONTAINER<COLLECTION::size_type> const& offsets; // Input.
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

#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Common/PtrVector.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "cetlib/map_vector.h"

#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

namespace art {
  // Forward declarations.
  class EDProductGetter;

  // Append container in to container out.
  template <typename CONTAINER>
  void concatContainers(CONTAINER& out, CONTAINER const& in);

  // 1.
  template <typename COLLECTION>
  void flattenCollections(std::vector<COLLECTION const*> const& in,
                          COLLECTION& out);

  // 2.
  template <typename COLLECTION, typename OFFSETS>
  void flattenCollections(std::vector<COLLECTION const*> const& in,
                          COLLECTION& out,
                          OFFSETS& offsets);

  // 3.
  template <typename T>
  void flattenCollections(std::vector<PtrVector<T> const*> const& in,
                          PtrVector<T>& out);

  // 4.
  template <typename T, typename OFFSETS>
  void flattenCollections(std::vector<PtrVector<T> const*> const& in,
                          PtrVector<T>& out,
                          OFFSETS& offsets);
} // namespace art

////////////////////////////////////////////////////////////////////////
// No user-serviceable parts below.
////////////////////////////////////////////////////////////////////////

namespace art::detail {
  // A. Verify a collection of PtrVector const*
  template <typename T>
  bool verifyPtrCollection(std::vector<art::PtrVector<T> const*> const& in);

  // B. Verify a collection (including PtrVector) of Ptrs.
  template <typename iterator>
  bool verifyPtrCollection(iterator beg,
                           iterator end,
                           art::ProductID id = {},
                           art::EDProductGetter const* getter = nullptr);
}

// A.
template <typename T>
bool
art::detail::verifyPtrCollection(
  std::vector<art::PtrVector<T> const*> const& in)
{
  return verifyPtrCollection(in.begin(), in.end());
}

// B.
template <typename iterator>
bool
art::detail::verifyPtrCollection(iterator const beg,
                                 iterator const end,
                                 art::ProductID const id,
                                 art::EDProductGetter const* getter)
{
  if (beg == end)
    return true;
  if (!id.isValid()) {
    id = (*beg).id();
  }
  if (!getter) {
    getter = (*beg).productGetter();
  }
  for (iterator i = beg; i != end; ++i) {
    if ((*i) != nullptr &&
        !((*i)->productGetter() && (*i)->productGetter() == getter &&
          (*i)->id().isValid() && (*i)->id() == id)) {
      return false;
    }
  }
  return true;
}

namespace art::detail {

  template <typename C>
  struct mix_offset {
    static size_t
    offset(C const& c)
    {
      return c.size();
    }
  };

  template <typename P>
  struct mix_offset<cet::map_vector<P>> {
    static size_t
    offset(cet::map_vector<P> const& mv)
    {
      return mv.delta();
    }
  };

  template <typename CONTAINER>
  struct TwoArgInsert {
    static void
    concatenate(CONTAINER& out, CONTAINER const& in)
    {
      out.insert(in.begin(), in.end());
    }
  };

  template <typename T>
  struct TwoArgInsert<cet::map_vector<T>> {
    using mv_t = cet::map_vector<T>;
    static void
    concatenate(mv_t& out, mv_t in)
    {
      // The offset is necessary for concatenating map_vectors so
      // that all elements will be preserved.
      auto const d = detail::mix_offset<mv_t>::offset(out);
      for (auto& pr : in) {
        pr.first = cet::map_vector_key{pr.first.asInt() + d};
      }
      // Because we can guarantee that entries will not overlap due
      // to using the offset above, we do not need to call
      // out.insert(...), which will unnecessarily merge entries.
      // It is the user's responsibility to ensure that the
      // map_vector entries are properly sorted.
      out.append(in.begin(), in.end());
    }
  };

  // Metaprogramming for two-argument insertion
  template <typename T, typename InIter = typename T::const_iterator>
  using two_arg_insert_t = decltype(
    std::declval<T>().insert(std::declval<InIter>(), std::declval<InIter>()));

  template <typename T, typename = void>
  struct has_two_arg_insert : std::false_type {};

  template <typename T>
  struct has_two_arg_insert<T, std::void_t<two_arg_insert_t<T>>>
    : std::true_type {};

  // Metaprogramming for three-argument insertion
  template <typename T,
            typename OutIter,
            typename InIter = typename T::const_iterator>
  using three_arg_insert_t =
    decltype(std::declval<T>().insert(std::declval<OutIter>(),
                                      std::declval<InIter>(),
                                      std::declval<InIter>()));

  template <typename T, typename OutIter, typename = void>
  struct has_three_arg_insert_t : std::false_type {};

  template <typename T, typename OutIter>
  struct has_three_arg_insert_t<T,
                                OutIter,
                                std::void_t<three_arg_insert_t<T, OutIter>>>
    : std::true_type {};

  template <typename T>
  constexpr bool has_three_arg_insert =
    has_three_arg_insert_t<T, typename T::iterator>::value ||
    has_three_arg_insert_t<T, typename T::const_iterator>::value;

} // namespace art::detail

template <typename CONTAINER>
void
art::concatContainers(CONTAINER& out, CONTAINER const& in)
{
  if constexpr (detail::has_two_arg_insert<CONTAINER>::value) {
    detail::TwoArgInsert<CONTAINER>::concatenate(out, in);
  } else {
    static_assert(detail::has_three_arg_insert<CONTAINER>);
    out.insert(out.end(), in.begin(), in.end());
  }
}

// 1.
template <typename COLLECTION>
void
art::flattenCollections(std::vector<COLLECTION const*> const& in,
                        COLLECTION& out)
{
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
art::flattenCollections(std::vector<COLLECTION const*> const& in,
                        COLLECTION& out,
                        OFFSETS& offsets)
{
  offsets.clear();
  offsets.reserve(in.size());
  typename COLLECTION::size_type current_offset{};
  for (auto collptr : in) {
    if (collptr == nullptr)
      continue;

    auto const delta = detail::mix_offset<COLLECTION>::offset(*collptr);
    offsets.push_back(current_offset);
    current_offset += delta;
  }
  flattenCollections<COLLECTION>(in, out); // 1.
}

// 3.
template <typename T>
void
art::flattenCollections(std::vector<PtrVector<T> const*> const& in,
                        PtrVector<T>& out)
{
  // Extra checks are required to verify that the PtrVectors are
  // compatible.
  if (!detail::verifyPtrCollection(in)) {
    throw Exception(errors::LogicError)
      << "Attempt to flatten incompatible PtrVectors "
      << "referring to different ProductIDs.\n";
  }
  flattenCollections<PtrVector<T>>(in, out); // 1
}

// 4.
template <typename T, typename OFFSETS>
void
art::flattenCollections(std::vector<PtrVector<T> const*> const& in,
                        PtrVector<T>& out,
                        OFFSETS& offsets)
{
  // Extra checks are required to verify that the PtrVectors are
  // compatible.
  if (!detail::verifyPtrCollection(in)) {
    throw Exception(errors::LogicError)
      << "Attempt to flatten incompatible PtrVectors "
      << "referring to different ProductIDs.\n";
  }
  flattenCollections<PtrVector<T>>(in, out, offsets); // 2.
}

#endif /* art_Persistency_Common_CollectionUtilities_h */

// Local Variables:
// mode: c++
// End:
