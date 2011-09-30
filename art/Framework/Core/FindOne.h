#ifndef art_Framework_Core_FindOne_h
#define art_Framework_Core_FindOne_h

#include "art/Framework/Core/detail/getAssnsHandle.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Persistency/Common/Assns.h"
#include "art/Persistency/Common/Ptr.h"
#include "art/Utilities/InputTag.h"
#include "art/Utilities/detail/metaprogramming.h"
#include "art/Utilities/ensurePointer.h"
#include "art/Utilities/pointersEqual.h"
#include "cpp0x/type_traits"

#include <vector>

namespace art {
  // General template
  template <typename ProdB, typename DATA = void>
  class FindOne;

  // Specialization.
  template <typename ProdB>
  class FindOne<ProdB, void>;

  namespace detail {

    struct ProductIDProvider;

    struct ConstantProductIDProvider;

    template <typename ProdA, typename ProdB, typename Data>
    class OneFinder;

    template <typename DATA, typename ASSNS, typename DATACOLL>
    typename std::enable_if < !std::is_void<DATA>::value >::type
    maybe_fill_data(ptrdiff_t assns_index,
                    ASSNS const & assns,
                    size_t data_index,
                    DATACOLL & data)
    {
      data[data_index] = &assns.data(assns_index);
    }

    template <typename DATA, typename ASSNS, typename DATACOLL>
    typename std::enable_if < std::is_void<DATA>::value >::type
    maybe_fill_data(ptrdiff_t,
                    ASSNS const &,
                    size_t,
                    DATACOLL &) { }
  }

  template <typename DATA, typename DATACOLL>
  typename std::enable_if < !std::is_void<DATA>::value >::type
  maybe_init_data(size_t size,
                  DATACOLL & data)
  {
    data.assign(size, 0);
  }

  template <typename DATA, typename DATACOLL>
  typename std::enable_if < std::is_void<DATA>::value >::type
  maybe_init_data(size_t,
                  DATACOLL &) { }
}

struct art::detail::ProductIDProvider {
  template <typename T>
  ProductID
  operator()(T const & t) const { return t.id(); }
};

class art::detail::ConstantProductIDProvider {
public:
  ConstantProductIDProvider(ProductID const & pid) : pid_(pid) { }
  template <typename T>
  ProductID const &
  operator()(T const &) const { return pid_; }
  ProductID const &
  operator()() const { return pid_; }
private:
  ProductID const & pid_;
};

template <typename ProdA, typename ProdB, typename Data>
class art::detail::OneFinder {
private:
  class Def { };
  //  typedef typename std::conditional<std::is_void<Data>::value, Def, typename FindOne<ProdB, Data>::dataColl_t>::type dataColl_t;
  typedef typename std::conditional<std::is_void<Data>::value, Def, std::vector<Data const *> >::type dataColl_t;

public:
  OneFinder(Event const & e, InputTag const & tag) : event_(e), assnsTag_(tag) { }

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
template <typename ProdA, typename ProdB, typename Data>
template <typename PidProvider, typename Acoll, typename Bcoll>
void
art::detail::OneFinder<ProdA, ProdB, Data>::
operator()(Acoll const & aColl, PidProvider const & pp, Bcoll & bColl) const
{
  Def dummy;
  (*this)(aColl, pp, bColl, dummy);
}

// 2.
template <typename ProdA, typename ProdB, typename Data>
template <typename PidProvider, typename Acoll, typename Bcoll>
typename std::enable_if <std::is_same<typename std::remove_cv<typename std::remove_pointer<typename Acoll::value_type>::type>::type, ProdA>::value >::type
art::detail::OneFinder<ProdA, ProdB, Data>::
operator()(Acoll const & aColl, PidProvider const & pp, Bcoll & bColl, dataColl_t & dColl) const
{
  Handle<Assns<ProdA, ProdB, Data> > assnsHandle(detail::getAssnsHandle<ProdA, ProdB, Data>(event_, assnsTag_));
  bColl.assign(aColl.size(), 0);
  maybe_init_data<Data>(aColl.size(), dColl);
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
      if (bColl[aIndex]) {
        throw Exception(errors::LogicError)
            << "Attempted to create a FindOne object for a one-many or many-many"
            << " association specified in collection "
            << assnsTag_
            << ".\n";
      }
      else {
        bColl[aIndex] = itAssns->second.get();
        maybe_fill_data<Data>(itAssns - beginAssns, *assnsHandle, aIndex, dColl);
      }
    }
  }
}

// 3.
template <typename ProdA, typename ProdB, typename Data>
template <typename PidProvider, typename Acoll, typename Bcoll>
typename std::enable_if<std::is_same<typename Acoll::value_type, art::Ptr<ProdA> >::value>::type
art::detail::OneFinder<ProdA, ProdB, Data>::
operator()(Acoll const & aColl, PidProvider const & pp, Bcoll & bColl, dataColl_t & dColl) const
{
  Handle<Assns<ProdA, ProdB, Data> > assnsHandle(detail::getAssnsHandle<ProdA, ProdB, Data>(event_, assnsTag_));
  bColl.assign(aColl.size(), 0);
  maybe_init_data<Data>(aColl.size(), dColl);
  for (typename Assns<ProdA, ProdB, Data>::assn_iterator
       beginAssns = assnsHandle->begin(),
       itAssns = beginAssns,
       endAssns = assnsHandle->end();
       itAssns != endAssns;
       ++itAssns) {
    size_t bIndex(0);
    typename Acoll::const_iterator
    i = aColl.begin(),
    e = aColl.end();
    // FIXME: Linear search: maybe we could do something more clever here?
    for (; i != e; ++i, ++bIndex) {
      if (itAssns->first.id() == pp(*i) &&
          pointersEqual(itAssns->first.get(),
                        ensurePointer<typename Acoll::value_type::const_pointer>(i))) {
        if (bColl[bIndex]) {
          throw Exception(errors::LogicError)
              << "Attempted to create a FindOne object for a one-many or many-many"
              << " association specified in collection "
              << assnsTag_
              << ".\n";
        }
        else {
          bColl[bIndex] = itAssns->second.get();
          maybe_fill_data<Data>(itAssns - beginAssns, *assnsHandle, bIndex, dColl);
          break;
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////
// Implementation of the specialization.
template <typename ProdB>
class art::FindOne<ProdB, void> {
public:
  typedef std::vector<ProdB const *> bColl_t;
  typedef typename bColl_t::value_type value_type;
  typedef typename bColl_t::size_type size_type;
  typedef typename bColl_t::difference_type difference_type;
  typedef typename bColl_t::const_reference const_reference;
  typedef typename bColl_t::reference reference;

  typedef ProdB assoc_t;

  template <typename ProdAColl>
  FindOne(Handle<ProdAColl> const & aCollection,
          Event const & e,
          InputTag const & tag);

  template <typename ProdA>
  FindOne(View<ProdA> const & view,
          Event const & e,
          InputTag const & tag);

  template <typename PtrProdAColl>
  FindOne(PtrProdAColl const & aPtrColl,
          Event const & e,
          InputTag const & tag);

  // Number of query results
  size_type size() const;

  // Associated item by index (not bounds-checked).
  value_type assoc(size_type i) const;

  // Associated item by index (bounds-checked).
  void get(size_type i, reference item) const;

  bool operator == (FindOne<ProdB, void> const & other) const;

private:
protected: // Hopefully temporary.
  FindOne() : bCollection_() { }
  bColl_t bCollection_;
};

template <typename ProdB, typename Data>
class art::FindOne : private art::FindOne<ProdB, void> {
private:
  typedef FindOne<ProdB, void> base;
public:
  typedef std::vector<Data const *> dataColl_t;
  typedef typename base::value_type value_type;
  typedef typename base::size_type size_type;
  typedef typename base::difference_type difference_type;
  typedef typename base::const_reference const_reference;
  typedef typename base::reference reference;
  typedef typename base::assoc_t assoc_t;

  typedef Data const * data_const_pointer;
  typedef Data const & data_const_reference;
  typedef Data & data_reference;

  typedef Data data_t;

  template <typename ProdAColl>
  FindOne(Handle<ProdAColl> const & aCollection,
          Event const & e,
          InputTag const & tag);

  template <typename ProdA>
  FindOne(View<ProdA> const & view,
          Event const & e,
          InputTag const & tag);

  template <typename PtrProdAColl>
  FindOne(PtrProdAColl const & aPtrColl,
          Event const & e,
          InputTag const & tag);

  using base::size;
  using base::assoc;
  using base::get;

  // Association extra-data object by index (not bounds-checked).
  data_const_pointer data(size_type i) const;

  // Associated item and extra-data object by index (bounds-checked).
  void get(size_type i, reference item, data_reference data) const;

  bool operator == (FindOne<ProdB, Data> const & other) const;

private:
  dataColl_t dataCollection_;
};

////////////////////////////////////////////////////////////////////////
// Base class implementation.
template <typename ProdB>
template <typename ProdAColl>
art::FindOne<ProdB, void>::
FindOne(Handle<ProdAColl> const & aCollection,
        Event const & e,
        InputTag const & tag)
  :
  bCollection_()
{
  typedef typename ProdAColl::value_type ProdA;
  detail::OneFinder<ProdA, ProdB, void> finder(e, tag);
  finder(*aCollection, detail::ConstantProductIDProvider(aCollection.id()), bCollection_);
}

template <typename ProdB>
template <typename ProdA>
art::FindOne<ProdB, void>::
FindOne(View<ProdA> const & view,
        Event const & e,
        InputTag const & tag)
  :
  bCollection_()
{
  typename View<ProdA>::collection_type const & aCollection(view.vals());
  detail::OneFinder<ProdA, ProdB, void> finder(e, tag);
  finder(aCollection, detail::ConstantProductIDProvider(view.id()), bCollection_);
}

template <typename ProdB>
template <typename PtrProdAColl>
art::FindOne<ProdB, void>::
FindOne(PtrProdAColl const & aPtrColl,
        Event const & e,
        InputTag const & tag)
  :
  bCollection_()
{
  typedef typename PtrProdAColl::value_type::value_type ProdA;
  detail::OneFinder<ProdA, ProdB, void> finder(e, tag);
  finder(aPtrColl, detail::ProductIDProvider(), bCollection_);
}

template <typename ProdB>
inline
typename art::FindOne<ProdB, void>::size_type
art::FindOne<ProdB, void>::size() const
{
  return bCollection_.size();
}

template <typename ProdB>
inline
typename art::FindOne<ProdB, void>::value_type
art::FindOne<ProdB, void>::assoc(size_type i) const
{
  return bCollection_[i];
}

template <typename ProdB>
inline
void
art::FindOne<ProdB, void>::get(size_type i, reference item) const
{
  item = bCollection_.at(i);
}

template <typename ProdB>
inline
bool
art::FindOne<ProdB, void>::operator == (FindOne<ProdB, void> const & other) const
{
  return bCollection_ == other.bCollection_;
}

////////////////////////////////////////////////////////////////////////
// Derived class implementation.
template <typename ProdB, typename Data>
template <typename ProdAColl>
art::FindOne<ProdB, Data>::FindOne(Handle<ProdAColl> const & aCollection,
                                   Event const & e,
                                   InputTag const & tag)
  :
  base(),
  dataCollection_()
{
  typedef typename ProdAColl::value_type ProdA;
  detail::OneFinder<ProdA, ProdB, Data> finder(e, tag);
  finder(*aCollection, detail::ConstantProductIDProvider(aCollection.id()), base::bCollection_, dataCollection_);
}

template <typename ProdB, typename Data>
template <typename ProdA>
art::FindOne<ProdB, Data>::FindOne(View<ProdA> const & view,
                                   Event const & e,
                                   InputTag const & tag)
  :
  base(),
  dataCollection_()
{
  typename View<ProdA>::collection_type const & aCollection(view.vals());
  detail::OneFinder<ProdA, ProdB, Data> finder(e, tag);
  finder(aCollection, detail::ConstantProductIDProvider(view.id()), base::bCollection_, dataCollection_);
}

template <typename ProdB, typename Data>
template <typename PtrProdAColl>
art::FindOne<ProdB, Data>::FindOne(PtrProdAColl const & aPtrColl,
                                   Event const & e,
                                   InputTag const & tag)
  :
  base(),
  dataCollection_()
{
  typedef typename PtrProdAColl::value_type::value_type ProdA;
  detail::OneFinder<ProdA, ProdB, Data> finder(e, tag);
  finder(aPtrColl, detail::ProductIDProvider(), base::bCollection_, dataCollection_);
}

template <typename ProdB, typename Data>
inline
typename art::FindOne<ProdB, Data>::data_const_pointer
art::FindOne<ProdB, Data>::data(size_type i) const
{
  return dataCollection_[i];
}

template <typename ProdB, typename Data>
inline
void
art::FindOne<ProdB, Data>::get(size_type i, reference item, data_reference data) const
{
  item = base::bCollection_.at(i);
  data = dataCollection_.at(i);
}

template <typename ProdB, typename Data>
inline
bool
art::FindOne<ProdB, Data>::operator == (FindOne<ProdB, Data> const & other) const
{
  return dataCollection_ == other.dataCollection_ &&
         this->base::operator==(other);
}
#endif /* art_Framework_Core_FindOne_h */

// Local Variables:
// mode: c++
// End:
