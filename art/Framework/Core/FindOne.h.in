#ifndef art_Framework_Core_FindOne_h
#define art_Framework_Core_FindOne_h
////////////////////////////////////////////////////////////////////////
// FindOne
//
// A smart query object used as the main way of accessing associated
// objects in a one-to-one association.
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
// the rest of the header is not for the faint of heart.
//
// Notes:
//
// * ProdB and Data are the only template arguments that must be
// specified when constructing a FindOne. Any other items are deducible
// from arguments.
//
// * An attempt to create a FindOne where one of the listed A objects
// actually has multiple B objects associated with it will result in an
// exception.
//
// * The FindOne needs a source of objects of type A, an event and an
// input tag corresponding to the underlying association collection from
// which to create itself.
//
// * When constructed, the FindOne will obtain and interrogate the
// correct Assns and provide access to the B (and/or D) object(s)
// associated with each supplied A object in the order in which the A
// objects were specified.
//
// * If the specified A does not have an associated B or D then the
// cet::maybe_ref will be invalid.
//
// * If the specified A has multiple associated Bs (or Ds) then an
// exception will be thrown: use FindMany instead.
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
// typedef typename std::vector<ProdB const *>::size_type size_type;
//
// Constructors.
//
// // From Handle to collection of A.
// FindOne<ProdB>(Handle<ProdAColl> const &,
//                Event const &,
//                InputTag const &);
// FindOne<ProdB, Data>(Handle<ProdAColl> const &,
//                      Event const &,
//                      InputTag const &);
//
// // From View<A>.
// FindOne<ProdB>(View<ProdA> const &,
//                Event const &,
//                InputTag const &);
// FindOne<ProdB, Data>(View<ProdA> const &,
//                      Event const &,
//                      InputTag const &);
//
// // From arbitrary sequence of Ptr<A>.
// FindOne<ProdB>(PtrProdAColl const &,
//                Event const &,
//                InputTag const &);
// FindOne<ProdB, Data>(PtrProdAColl const &,
//                      Event const &,
//                      InputTag const &);
//
// Modifiers.
//
// <NONE>.
//
// Accessors.
//
// size_type size() const;
// cet::maybe_ref<assoc_t const> at(size_type) const;
// cet::maybe_ref<data_t const> data(size_type) const;
// void get(size_type,
//          cet::maybe_ref<assoc_t const> &) const;
// void get(size_type,
//          cet::maybe_ref<assoc_t const> &,
//          cet::maybe_ref<data_t const> &)
//   const; // *Must* be used for FindOne<ProdB, Data>.
//
// Comparison operations.
//
// bool operator == (FindOne const & other) const;
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

  // Associated item by index (bounds-checked).
  cet::maybe_ref<assoc_t const> at(size_type i) const;
  void get(size_type i, cet::maybe_ref<assoc_t const> & item) const;

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
  using base::at;
  using base::get;

  // Association extra-data object by index (bounds-checked).
  cet::maybe_ref<Data const> data(size_type i) const;

  // Associated item and extra-data object by index (bounds-checked).
  void get(size_type i,
           cet::maybe_ref<assoc_t const> & item,
           cet::maybe_ref<Data const> & data) const;

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
  detail::IPRHelper<ProdA, ProdB, void, void> finder(e, tag);
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
  detail::IPRHelper<ProdA, ProdB, void, void> finder(e, tag);
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
  detail::IPRHelper<ProdA, ProdB, void, void> finder(e, tag);
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
cet::maybe_ref<typename art::FindOne<ProdB, void>::assoc_t const>
art::FindOne<ProdB, void>::at(size_type i) const
{
  return cet::maybe_ref<assoc_t const>(*bCollection_.at(i));
}

template <typename ProdB>
inline
void
art::FindOne<ProdB, void>::
get(size_type i, cet::maybe_ref<assoc_t const> & item) const
{
  item.reseat(*bCollection_.at(i));
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
  detail::IPRHelper<ProdA, ProdB, Data, dataColl_t> finder(e, tag);
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
  detail::IPRHelper<ProdA, ProdB, Data, dataColl_t> finder(e, tag);
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
  detail::IPRHelper<ProdA, ProdB, Data, dataColl_t> finder(e, tag);
  finder(aPtrColl, detail::ProductIDProvider(), base::bCollection(), dataCollection_);
}

template <typename ProdB, typename Data>
inline
cet::maybe_ref<Data const>
art::FindOne<ProdB, Data>::data(size_type i) const
{
  return cet::maybe_ref<Data const>(*dataCollection_.at(i));
}

template <typename ProdB, typename Data>
inline
void
art::FindOne<ProdB, Data>::
get(size_type i,
    cet::maybe_ref<assoc_t const> & item,
    cet::maybe_ref<Data const> & data) const
{
  base::get(i, item);
  data.reseat(*dataCollection_.at(i));
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
