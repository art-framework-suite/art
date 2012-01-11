#ifndef art_Framework_Core_FindMany_h
#define art_Framework_Core_FindMany_h
////////////////////////////////////////////////////////////////////////
// FindMany
//
// A smart query object used as the main way of accessing associated
// objects in an association (onoe-to-one, one-to-many or many-to-many).
//
// Given an Assns associating A with B (or B with A) (possibly with an
// associated data object D) and a sequence ACOLL of A objects to be
// found in the Assns, allow indexed access to the B and/or D objects
// associated with the A objects in ACOLL.
//
////////////////////////////////////
// Interface.
//////////
//
// For ease of understanding, the interface is presented here; reading
// the rest of the header is not for the faint of heart. Compare with
// the corresponding interface for FindOne.
//
// Notes:
//
// * ProdB and Data are the only template arguments that must be
// specified when constructing a FindMany. Any other items are deducible
// from arguments.
//
// * The FindMany needs a source of objects of type A, an event and an
// input tag corresponding to the underlying association collection from
// which to create itself.
//
// * When constructed, the FindMany will obtain and interrogate the
// correct Assns and provide access to the B (and/or D) object(s)
// associated with each supplied A object in the order in which the A
// objects were specified.
//
// * If the specified A does not have an associated B or D then the
// vector will be empty.
//
// * If the required association collection has an extra data object D
// with each association then it *must* be specified as a template
// argument, even if it is not relevant to the current query.
//
// * *All* indexed accessors (at(), data(), get()) are bounds-checked.
//
// Useful typedefs.
//
// typedef ProdB assoc_t;
// typedef Data data_t;
// typedef std::vector<assoc_t const *> value_type;
// typedef typename std::vector<value_type>::size_type size_type;
// typedef value_type const & const_reference;
// typedef value_type & reference;
// typedef typename std::vector<data_t const *> const & data_const_reference;
// typedef typename std::vector<data_t const *> & data_reference;
//
// Constructors.
//
// // From Handle to collection of A.
// FindMany<ProdB>(Handle<ProdAColl> const &,
//                 Event const &,
//                 InputTag const &);
// FindMany<ProdB, Data>(Handle<ProdAColl> const &,
//                       Event const &,
//                       InputTag const &);
//
// // From View<A>.
// FindMany<ProdB>(View<ProdA> const &,
//                 Event const &,
//                 InputTag const &);
// FindMany<ProdB, Data>(View<ProdA> const &,
//                       Event const &,
//                       InputTag const &);
//
// // From arbitrary sequence of Ptr<A>.
// FindMany<ProdB>(PtrProdAColl const &,
//                 Event const &,
//                 InputTag const &);
// FindMany<ProdB, Data>(PtrProdAColl const &,
//                       Event const &,
//                       InputTag const &);
//
// Modifiers.
//
// <NONE>.
//
// Accessors.
//
// size_type size() const;
// const_reference at(size_type) const;
// const_data_reference data(size_type) const;
// size_type get(size_type,
//               reference)
//   const; // Returns number of elements appended.
// size_type get(size_type,
//               reference,
//               data_reference)
//   const; // *Must* be used for FindMany<ProdB, Data>.
//
// Comparison operations.
//
// bool operator == (FindMany const & other) const;
//
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/detail/IPRHelper.h"
#include "art/Framework/Core/detail/ProductIDProvider.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Persistency/Common/Assns.h"
#include "art/Persistency/Common/Ptr.h"
#include "art/Utilities/InputTag.h"
#include "cetlib/maybe_ref.h"

#include <vector>

namespace art {
  // General template
  template <typename ProdB, typename DATA = void>
  class FindMany;

  // Specialization.
  template <typename ProdB>
  class FindMany<ProdB, void>;
}

////////////////////////////////////////////////////////////////////////
// Implementation of the specialization.
template <typename ProdB>
class art::FindMany<ProdB, void> {
public:
  typedef std::vector<std::vector<ProdB const *> > bColl_t;
  typedef typename bColl_t::value_type value_type;
  typedef typename bColl_t::size_type size_type;
  typedef typename bColl_t::difference_type difference_type;
  typedef typename bColl_t::const_reference const_reference;
  typedef typename bColl_t::reference reference;

  typedef ProdB assoc_t;

  template <typename ProdAColl>
  FindMany(Handle<ProdAColl> const & aCollection,
           Event const & e,
           InputTag const & tag);

  template <typename ProdA>
  FindMany(View<ProdA> const & view,
           Event const & e,
           InputTag const & tag);

  template <typename PtrProdAColl>
  FindMany(PtrProdAColl const & aPtrColl,
           Event const & e,
           InputTag const & tag);

  // Number of query results
  size_type size() const;

  // Associated item by index (bounds-checked).
  const_reference at(size_type i) const;
  size_type get(size_type i, reference item) const;

  bool operator == (FindMany<ProdB, void> const & other) const;

protected:
  FindMany() : bCollection_() { }
  bColl_t & bCollection() { return bCollection_; }

private:
  bColl_t bCollection_;
};

template <typename ProdB, typename Data>
class art::FindMany : private art::FindMany<ProdB, void> {
private:
  typedef FindMany<ProdB, void> base;
public:
  typedef std::vector<std::vector<Data const *> > dataColl_t;
  typedef typename base::value_type value_type;
  typedef typename base::size_type size_type;
  typedef typename base::difference_type difference_type;
  typedef typename base::const_reference const_reference;
  typedef typename base::reference reference;
  typedef typename base::assoc_t assoc_t;

  typedef typename dataColl_t::const_pointer data_const_pointer;
  typedef typename dataColl_t::const_reference data_const_reference;
  typedef typename dataColl_t::reference data_reference;

  typedef Data data_t;

  template <typename ProdAColl>
  FindMany(Handle<ProdAColl> const & aCollection,
           Event const & e,
           InputTag const & tag);

  template <typename ProdA>
  FindMany(View<ProdA> const & view,
           Event const & e,
           InputTag const & tag);

  template <typename PtrProdAColl>
  FindMany(PtrProdAColl const & aPtrColl,
           Event const & e,
           InputTag const & tag);

  using base::size;
  using base::at;
  using base::get;

  // Association extra-data object by index (bounds-checked).
  data_const_reference data(size_type i) const;

  // Associated item and extra-data object by index (bounds-checked).
  size_type get(size_type i, reference item, data_reference data) const;

  bool operator == (FindMany<ProdB, Data> const & other) const;

private:
  dataColl_t dataCollection_;
};

////////////////////////////////////////////////////////////////////////
// Base class implementation.
template <typename ProdB>
template <typename ProdAColl>
art::FindMany<ProdB, void>::
FindMany(Handle<ProdAColl> const & aCollection,
         Event const & e,
         InputTag const & tag)
  :
  bCollection_()
{
  typedef typename ProdAColl::value_type ProdA;
  detail::IPRHelper<ProdA, ProdB, void, void> finder(e, tag);
  finder(*aCollection, detail::ConstantProductIDProvider(aCollection.id()), bCollection_);
}

template <typename ProdB>
template <typename ProdA>
art::FindMany<ProdB, void>::
FindMany(View<ProdA> const & view,
         Event const & e,
         InputTag const & tag)
  :
  bCollection_()
{
  typename View<ProdA>::collection_type const & aCollection(view.vals());
  detail::IPRHelper<ProdA, ProdB, void, void> finder(e, tag);
  finder(aCollection, detail::ConstantProductIDProvider(view.id()), bCollection_);
}

template <typename ProdB>
template <typename PtrProdAColl>
art::FindMany<ProdB, void>::
FindMany(PtrProdAColl const & aPtrColl,
         Event const & e,
         InputTag const & tag)
  :
  bCollection_()
{
  typedef typename PtrProdAColl::value_type::value_type ProdA;
  detail::IPRHelper<ProdA, ProdB, void, void> finder(e, tag);
  finder(aPtrColl, detail::ProductIDProvider(), bCollection_);
}

template <typename ProdB>
inline
typename art::FindMany<ProdB, void>::size_type
art::FindMany<ProdB, void>::size() const
{
  return bCollection_.size();
}

template <typename ProdB>
inline
typename art::FindMany<ProdB, void>::const_reference
art::FindMany<ProdB, void>::at(size_type i) const
{
  return bCollection_.at(i);
}

template <typename ProdB>
inline
typename art::FindMany<ProdB, void>::size_type
art::FindMany<ProdB, void>::get(size_type i, reference item) const
{
  reference ref(bCollection_.at(i));
  item.insert(item.end(), ref.begin(), ref.end());
  return ref.size();
}

template <typename ProdB>
inline
bool
art::FindMany<ProdB, void>::operator == (FindMany<ProdB, void> const & other) const
{
  return bCollection_ == other.bCollection_;
}

////////////////////////////////////////////////////////////////////////
// Derived class implementation.
template <typename ProdB, typename Data>
template <typename ProdAColl>
art::FindMany<ProdB, Data>::FindMany(Handle<ProdAColl> const & aCollection,
                                     Event const & e,
                                     InputTag const & tag)
  :
  base(),
  dataCollection_()
{
  typedef typename ProdAColl::value_type ProdA;
  detail::IPRHelper<ProdA, ProdB, Data, dataColl_t> finder(e, tag);
  finder(*aCollection, detail::ConstantProductIDProvider(aCollection.id()), base::bCollection(), dataCollection_);
}

template <typename ProdB, typename Data>
template <typename ProdA>
art::FindMany<ProdB, Data>::FindMany(View<ProdA> const & view,
                                     Event const & e,
                                     InputTag const & tag)
  :
  base(),
  dataCollection_()
{
  typename View<ProdA>::collection_type const & aCollection(view.vals());
  detail::IPRHelper<ProdA, ProdB, Data, dataColl_t> finder(e, tag);
  finder(aCollection, detail::ConstantProductIDProvider(view.id()), base::bCollection(), dataCollection_);
}

template <typename ProdB, typename Data>
template <typename PtrProdAColl>
art::FindMany<ProdB, Data>::FindMany(PtrProdAColl const & aPtrColl,
                                     Event const & e,
                                     InputTag const & tag)
  :
  base(),
  dataCollection_()
{
  typedef typename PtrProdAColl::value_type::value_type ProdA;
  detail::IPRHelper<ProdA, ProdB, Data, dataColl_t> finder(e, tag);
  finder(aPtrColl, detail::ProductIDProvider(), base::bCollection(), dataCollection_);
}

template <typename ProdB, typename Data>
inline
typename art::FindMany<ProdB, Data>::data_const_reference
art::FindMany<ProdB, Data>::data(size_type i) const
{
  return dataCollection_.at(i);
}

template <typename ProdB, typename Data>
inline
typename art::FindMany<ProdB, Data>::size_type
art::FindMany<ProdB, Data>::get(size_type i, reference item, data_reference data) const
{
  size_type result = base::get(i, item);
  data_reference ref(dataCollection_.at(i));
  data.insert(data.end(), ref.begin(), ref.end());;
  return result;
}

template <typename ProdB, typename Data>
inline
bool
art::FindMany<ProdB, Data>::operator == (FindMany<ProdB, Data> const & other) const
{
  return dataCollection_ == other.dataCollection_ &&
         this->base::operator==(other);
}
#endif /* art_Framework_Core_FindMany_h */

// Local Variables:
// mode: c++
// End:
