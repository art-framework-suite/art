#ifndef art_Framework_Core_detail_IPRHelper_h
#define art_Framework_Core_detail_IPRHelper_h

// Helper class and associated gubbins for populating the FindOne and
// FindMany query objects for inter-product references.

#include "art/Framework/Core/detail/ProductIDProvider.h"
#include "art/Framework/Core/detail/getAssnsHandle.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Persistency/Common/Assns.h"
#include "art/Persistency/Common/Ptr.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/InputTag.h"
#include "art/Utilities/ensurePointer.h"
#include "art/Utilities/pointersEqual.h"
#include "cpp0x/type_traits"

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
      typename std::enable_if<std::is_same<typename Bcoll::value_type, Ptr<ProdB> >::value>::type
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
  IPRHelper(Event const & e, InputTag const & tag) : event_(e), assnsTag_(tag) { }

  // 1. Dispatches to one of the other two: use when dColl not wanted.
  template <typename PidProvider, typename Acoll, typename Bcoll>
  void
  operator()(Acoll const & aColl, PidProvider const & pp, Bcoll & bColl) const;

  // 2. Algorithm useful when index of Acoll == index of desired Ptr in Assns.
  template <typename PidProvider, typename Acoll, typename Bcoll>
  typename std::enable_if <std::is_same<typename std::remove_cv<typename std::remove_pointer<typename Acoll::value_type>::type>::type, ProdA>::value >::type
  operator()(Acoll const & aColl, PidProvider const & pp, Bcoll & bColl, dataColl_t & dColl) const;

  // 3. Algorithm useful when dealing with collections of Ptrs.
  template <typename PidProvider, typename Acoll, typename Bcoll>
  typename std::enable_if<std::is_same<typename Acoll::value_type, Ptr<ProdA> >::value>::type
  operator()(Acoll const & aColl, PidProvider const & pp, Bcoll & bColl, dataColl_t & dColl) const;

private:
  Event const & event_;
  InputTag const & assnsTag_;
};

// 1.
template <typename ProdA, typename ProdB, typename Data, typename DATACOLL>
template <typename PidProvider, typename Acoll, typename Bcoll>
void
art::detail::IPRHelper<ProdA, ProdB, Data, DATACOLL>::
operator()(Acoll const & aColl, PidProvider const & pp, Bcoll & bColl) const
{
  IPRHelperDef dummy;
  (*this)(aColl, pp, bColl, dummy);
}

// 2.
template <typename ProdA, typename ProdB, typename Data, typename DATACOLL>
template <typename PidProvider, typename Acoll, typename Bcoll>
typename std::enable_if <std::is_same<typename std::remove_cv<typename std::remove_pointer<typename Acoll::value_type>::type>::type, ProdA>::value >::type
art::detail::IPRHelper<ProdA, ProdB, Data, DATACOLL>::
operator()(Acoll const & aColl, PidProvider const & pp, Bcoll & bColl, dataColl_t & dColl) const
{
  detail::BcollHelper<ProdB> bh(assnsTag_);
  detail::DataCollHelper<Data> dh(assnsTag_);
  Handle<Assns<ProdA, ProdB, Data> > assnsHandle(detail::getAssnsHandle<ProdA, ProdB, Data>(event_, assnsTag_));
  bh.init(aColl.size(), bColl);
  dh.init(aColl.size(), dColl);
  for (typename Assns<ProdA, ProdB, Data>::assn_iterator
       beginAssns = assnsHandle->begin(),
       itAssns = beginAssns,
       endAssns = assnsHandle->end();
       itAssns != endAssns;
       ++itAssns) {
    size_t aIndex(itAssns->first.key());
    typename Acoll::const_iterator itA = aColl.begin();
    std::advance(itA, aIndex);
    if (itAssns->first.id() == pp() &&
        pointersEqual(itAssns->first.get(), ensurePointer<typename Assns<ProdA, ProdB, Data>::assn_iterator::value_type::first_type::const_pointer>(itA))) {
      bh.fill(aIndex, itAssns->second, bColl);
      dh.fill(itAssns - beginAssns, *assnsHandle, aIndex, dColl);
    }
  }
}

// 3.
template <typename ProdA, typename ProdB, typename Data, typename DATACOLL>
template <typename PidProvider, typename Acoll, typename Bcoll>
typename std::enable_if<std::is_same<typename Acoll::value_type, art::Ptr<ProdA> >::value>::type
art::detail::IPRHelper<ProdA, ProdB, Data, DATACOLL>::
operator()(Acoll const & aColl, PidProvider const & pp, Bcoll & bColl, dataColl_t & dColl) const
{
  detail::BcollHelper<ProdB> bh(assnsTag_);
  detail::DataCollHelper<Data> dh(assnsTag_);
  Handle<Assns<ProdA, ProdB, Data> > assnsHandle(detail::getAssnsHandle<ProdA, ProdB, Data>(event_, assnsTag_));
  bh.init(aColl.size(), bColl);
  dh.init(aColl.size(), dColl);
  for (typename Assns<ProdA, ProdB, Data>::assn_iterator
       beginAssns = assnsHandle->begin(),
       itAssns = beginAssns,
       endAssns = assnsHandle->end();
       itAssns != endAssns;
       ++itAssns) {
    size_t bIndex(0);
    // FIXME: Linear search: maybe we could do something more clever here?
    for (typename Acoll::const_iterator
         i = aColl.begin(),
         e = aColl.end();
         i != e;
         ++i, ++bIndex) {
      if (itAssns->first.id() == pp(*i) &&
          pointersEqual(itAssns->first.get(),
                        ensurePointer<typename Acoll::value_type::const_pointer>(i))) {
        bh.fill(bIndex, itAssns->second, bColl);
        dh.fill(itAssns - beginAssns, *assnsHandle, bIndex, dColl);
        break;
      }
    }
  }
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
typename std::enable_if<std::is_same<typename Bcoll::value_type, art::Ptr<ProdB> >::value>::type
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
