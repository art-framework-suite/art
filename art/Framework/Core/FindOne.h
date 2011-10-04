#ifndef art_Framework_Core_FindOne_h
#define art_Framework_Core_FindOne_h

#include "art/Framework/Core/detail/IPRHelper.h"
#include "art/Framework/Core/detail/ProductIDProvider.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Persistency/Common/Assns.h"
#include "art/Persistency/Common/Ptr.h"
#include "art/Utilities/InputTag.h"

#include <vector>

namespace art {
  // General template
  template <typename ProdB, typename DATA = void>
  class FindOne;

  // Specialization.
  template <typename ProdB>
  class FindOne<ProdB, void>;
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

protected: 
  FindOne() : bCollection_() { }
  bColl_t & bCollection() { return bCollection_; }

private:
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
  detail::IPRHelper<ProdA, ProdB, void> finder(e, tag);
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
  detail::IPRHelper<ProdA, ProdB, void> finder(e, tag);
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
  detail::IPRHelper<ProdA, ProdB, void> finder(e, tag);
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
  detail::IPRHelper<ProdA, ProdB, Data> finder(e, tag);
  finder(*aCollection, detail::ConstantProductIDProvider(aCollection.id()), base::bCollection(), dataCollection_);
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
  detail::IPRHelper<ProdA, ProdB, Data> finder(e, tag);
  finder(aCollection, detail::ConstantProductIDProvider(view.id()), base::bCollection(), dataCollection_);
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
  detail::IPRHelper<ProdA, ProdB, Data> finder(e, tag);
  finder(aPtrColl, detail::ProductIDProvider(), base::bCollection(), dataCollection_);
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
  base::get(i, item);
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
