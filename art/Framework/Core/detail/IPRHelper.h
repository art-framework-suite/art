#ifndef art_Framework_Core_detail_IPRHelper_h
#define art_Framework_Core_detail_IPRHelper_h

// Helper class and associated gubbins for populating the FindOne and
// FindMany query objects for inter-product references.

#include "art/Framework/Core/detail/getAssnsHandle.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Persistency/Common/Assns.h"
#include "art/Persistency/Common/Ptr.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/InputTag.h"
#include "art/Utilities/ensurePointer.h"
#include "art/Utilities/pointersEqual.h"

#include <type_traits>
#include <unordered_map>

namespace art {
  namespace detail {

    class IPRHelperDef { };

    template <typename ProdA, typename ProdB, typename Data, typename DATACOLL>
    class IPRHelper;

    template <typename DATA>
    class DataCollHelper {
    public:
      DataCollHelper(InputTag const & assnsTag);

      void init(size_t size,
                std::vector<DATA const *> & data) const;
      template <typename ASSNS>
      void fill(ptrdiff_t assns_index,
                ASSNS const & assns,
                size_t data_index,
                std::vector<DATA const *> & data) const;

      void init(size_t size,
                std::vector<std::vector<DATA const *> > & data) const;
      template <typename ASSNS>
      void fill(ptrdiff_t assns_index,
                ASSNS const & assns,
                size_t data_index,
                std::vector<std::vector<DATA const *> > & data) const;

      void init(size_t, IPRHelperDef &) const;
      template <typename ASSNS>
      void fill(ptrdiff_t,
                ASSNS const &,
                size_t,
                IPRHelperDef &) const;

    private:
      InputTag const & assnsTag_;
    };

    template <typename ProdB>
    class BcollHelper {
    public:
      BcollHelper(InputTag const & assnsTag);
      template <typename Bcoll>
      void init(size_t size, Bcoll & bColl) const;

      template <typename Bcoll>
      typename std::enable_if<std::is_same<typename Bcoll::value_type, ProdB const *>::value>::type
      fill(size_t index,
           Ptr<ProdB> const & item,
           Bcoll & bColl) const;

      template <typename Bcoll>
      typename std::enable_if<std::is_convertible<typename Bcoll::value_type, Ptr<ProdB> >::value>::type
      fill(size_t index,
           Ptr<ProdB> const & item,
           Bcoll & bColl) const;

      template <typename Bcoll>
      void init(size_t size, std::vector<Bcoll> & bColls) const;

      template <typename Bcoll>
      typename std::enable_if<std::is_same<typename Bcoll::value_type, ProdB const *>::value>::type
      fill(size_t index,
           Ptr<ProdB> const & item,
           std::vector<Bcoll> & bColls) const;

      template <typename Bcoll>
      typename std::enable_if<std::is_convertible<typename Bcoll::value_type, Ptr<ProdB> >::value>::type
      fill(size_t index,
           Ptr<ProdB> const & item,
           std::vector<Bcoll> & bColls) const;

    private:
      InputTag const & assnsTag_;
    };
  }
}

template <typename ProdA, typename ProdB, typename Data, typename DATACOLL>
class art::detail::IPRHelper {
private:

  typedef typename std::conditional<std::is_void<Data>::value, IPRHelperDef, DATACOLL>::type dataColl_t;

public:
  typedef std::shared_ptr<cet::exception const> shared_exception_t;

  IPRHelper(Event const & e, InputTag const & tag) : event_(e), assnsTag_(tag) { }

  // 1. When dColl not wanted.
  template <typename Acoll, typename Bcoll>
  shared_exception_t
  operator()(Acoll const & aColl, Bcoll & bColl) const;

  // 2. Algorithm useful when dealing with collections of Ptrs.
  template <typename Acoll, typename Bcoll>
  shared_exception_t
  operator()(Acoll const & aColl, Bcoll & bColl, dataColl_t & dColl) const;

private:
  Event const & event_;
  InputTag const & assnsTag_;
};

// 1.
template <typename ProdA, typename ProdB, typename Data, typename DATACOLL>
template <typename Acoll, typename Bcoll>
inline
auto
art::detail::IPRHelper<ProdA, ProdB, Data, DATACOLL>::
operator()(Acoll const & aColl, Bcoll & bColl) const
-> shared_exception_t
{
  IPRHelperDef dummy;
  return (*this)(aColl, bColl, dummy);
}

// 2.
////////////////////////////////////////////////////////////////////////
// Implementation notes.
//
// The current implementation does not verify the that ProductID of the
// item in the association collection matches that of the item in the
// reference collection before attempting to dereference its Ptr
// (although it does verify ptr.isAvailable()). This means that in the
// case where an association collection refers to multiple vailable
// AProd collections, all of those collections will be read from file
// even if the reference collection does not include items from one or
// more of those AProd collections.
//
// If one were to provide an implementation that did this, one would
// change the unordered_multimap to key on the full ptr instead of the
// pointer. This would involve defining a hashing function, something
// along the lines of:
//
// auto hasher = [](typename Acoll::value_type::const_pointer const & ptr)
//               { return std::hash((ptr.key() && 0xffffffff) +
//                                  (ptr.id().productIndex() << 32) +
//                                  (ptr.id().processIndex() << 48)); };
//
// However, it would be problematic to do the lookup if the reference
// item was not in fact a Ptr. Maybe it would be relatively efficient if
// one were able to do a lookup in the table against an entity not a Ptr
// for which I could write a comparison function that compared the
// ProductID and only if they matched, the pointer with suitable get().
//
// For now however, no-one has requested this,
////////////////////////////////////////////////////////////////////////
template <typename ProdA, typename ProdB, typename Data, typename DATACOLL>
template <typename Acoll, typename Bcoll>
auto
art::detail::IPRHelper<ProdA, ProdB, Data, DATACOLL>::
operator()(Acoll const & aColl, Bcoll & bColl, dataColl_t & dColl) const
-> shared_exception_t
{
  detail::BcollHelper<ProdB> bh(assnsTag_);
  detail::DataCollHelper<Data> dh(assnsTag_);
  Handle<Assns<ProdA, ProdB, Data> >
    assnsHandle(detail::getAssnsHandle<ProdA, ProdB, Data>(event_, assnsTag_));
  if (!assnsHandle.isValid()) {
    return assnsHandle.whyFailed(); // Failed to get Assns product.
  }
  bh.init(aColl.size(), bColl);
  dh.init(aColl.size(), dColl);
  // Answer cache.
  std::unordered_multimap<typename Ptr<ProdA>::const_pointer,
    std::pair<Ptr<ProdB>, ptrdiff_t> > lookupCache;
  ptrdiff_t counter { 0 };
  for (auto const & apair : *assnsHandle) {
    if (apair.first.isAvailable()) {
      lookupCache.emplace(apair.first.get(),
                          typename decltype(lookupCache)::mapped_type(apair.second, counter));
    }
    ++counter;
  }
  // Now use the cache.
  size_t bIndex { 0 };
  for (typename Acoll::const_iterator
         i = aColl.begin(),
         e = aColl.end();
       i != e;
       ++i, ++bIndex) {
    auto foundItems = lookupCache.equal_range(ensurePointer<typename Ptr<ProdA>::const_pointer>(i));
    if (foundItems.first != lookupCache.cend()) {
      std::for_each(foundItems.first, foundItems.second,
                    [&bh, &dh, &bColl, bIndex, &assnsHandle, &dColl]
                    (typename decltype(lookupCache)::const_reference itemPair)
                    {
                      bh.fill(bIndex, itemPair.second.first, bColl);
                      dh.fill(itemPair.second.second, *assnsHandle, bIndex, dColl);
                    });
    }
  }
  return shared_exception_t();
}

template <typename DATA>
inline
art::detail::DataCollHelper<DATA>::
DataCollHelper(InputTag const & assnsTag)
  :
  assnsTag_(assnsTag)
{
}

template <typename DATA>
inline
void
art::detail::DataCollHelper<DATA>::
init(size_t size,
     std::vector<DATA const *> & data) const
{
  data.assign(size, 0);
}

template <typename DATA>
template <typename ASSNS>
inline
void
art::detail::DataCollHelper<DATA>::
fill(ptrdiff_t assns_index,
     ASSNS const & assns,
     size_t data_index,
     std::vector<DATA const *> & data) const
{
  // Check not necessary as this always occurs after bColl filling, which is checked.
  data[data_index] = &assns.data(assns_index);
}

template <typename DATA>
inline
void
art::detail::DataCollHelper<DATA>::
init(size_t size,
     std::vector<std::vector<DATA const *> > & data) const
{
  data.resize(size);
}

template <typename DATA>
template <typename ASSNS>
inline
void
art::detail::DataCollHelper<DATA>::
fill(ptrdiff_t assns_index,
     ASSNS const & assns,
     size_t data_index,
     std::vector<std::vector<DATA const *> > & data) const
{
 if (&assns.data(assns_index)) {
    data[data_index].push_back(&assns.data(assns_index));
 }
}

template <typename DATA>
inline
void
art::detail::DataCollHelper<DATA>::
init(size_t, IPRHelperDef &) const
{
}

template <typename DATA>
template <typename ASSNS>
inline
void
art::detail::DataCollHelper<DATA>::
fill(ptrdiff_t,
     ASSNS const &,
     size_t,
     IPRHelperDef &) const
{
}

template <typename ProdB>
inline
art::detail::BcollHelper<ProdB>::
BcollHelper(InputTag const & assnsTag)
  :
  assnsTag_(assnsTag)
{
}

template <typename ProdB>
template <typename Bcoll>
inline
void
art::detail::BcollHelper<ProdB>::
init(size_t size, Bcoll & bColl) const
{
  bColl.assign(size, 0);
}

template <typename ProdB>
template <typename Bcoll>
inline
typename std::enable_if<std::is_same<typename Bcoll::value_type, ProdB const *>::value>::type
art::detail::BcollHelper<ProdB>::
fill(size_t index,
     Ptr<ProdB> const & item,
     Bcoll & bColl) const
{
  if (bColl[index]) {
    throw Exception(errors::LogicError)
        << "Attempted to create a FindOne object for a one-many or many-many"
        << " association specified in collection "
        << assnsTag_
        << ".\n";
  }
  bColl[index] =  item.isAvailable() ? item.get() : nullptr;
}

template <typename ProdB>
template <typename Bcoll>
inline
typename std::enable_if<std::is_convertible<typename Bcoll::value_type, art::Ptr<ProdB> >::value>::type
art::detail::BcollHelper<ProdB>::
fill(size_t index,
     Ptr<ProdB> const & item,
     Bcoll & bColl) const
{
  if (bColl[index]) {
    throw Exception(errors::LogicError)
        << "Attempted to create a FindOne object for a one-many or many-many"
        << " association specified in collection "
        << assnsTag_
        << ".\n";
  }
  bColl[index] = item;
}

template <typename ProdB>
template <typename Bcoll>
inline
void
art::detail::BcollHelper<ProdB>::
init(size_t size, std::vector<Bcoll> & bColls) const
{
  bColls.resize(size);
}

template <typename ProdB>
template <typename Bcoll>
inline
typename std::enable_if<std::is_same<typename Bcoll::value_type, ProdB const *>::value>::type
art::detail::BcollHelper<ProdB>::
fill(size_t index,
     Ptr<ProdB> const & item,
     std::vector<Bcoll> & bColls) const
{
  if (item) {
    bColls[index].push_back( item.isAvailable() ? item.get() : nullptr);
  }
}

template <typename ProdB>
template <typename Bcoll>
inline
typename std::enable_if<std::is_convertible<typename Bcoll::value_type, art::Ptr<ProdB> >::value>::type
art::detail::BcollHelper<ProdB>::
fill(size_t index,
     Ptr<ProdB> const & item,
     std::vector<Bcoll> & bColls) const
{
  if (item) {
     bColls[index].push_back(item);
  }
}

#endif /* art_Framework_Core_detail_IPRHelper_h */

// Local Variables:
// mode: c++
// End:
