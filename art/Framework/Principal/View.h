#ifndef art_Framework_Principal_View_h
#define art_Framework_Principal_View_h

//
// The class template View<T> provides a means to obtain pointers (of
// type T const*) into an arbitrary collection in an Event.
//
// A View<T> is *valid* if it refers to a product in an Event. Default
// constructed Views are not valid. A valid View may still contain an
// empty vector; this means that either the referenced collection was
// empty, or that the View's vector was emptied after the View was
// created.
//
// While View<T> is not a persistent class, one can fill a
// PtrVector<T> from a View<T>, as long as no new pointers have been
// added to the View<T>.
//


#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/PtrVector.h"

template <class T>
class art::View  {
public:
  typedef std::vector<T const*> collection_type;
  typedef typename collection_type::value_type value_type;
  typedef typename collection_type::const_iterator const_iterator;
  typedef typename collection_type::iterator iterator;
  typedef typename collection_type::size_type size_type;

  View() : vals_(), id_(), prod_(0) { }
  collection_type&       vals()       { return vals_; }
  collection_type const& vals() const { return vals_; }

  // return true if this view has been populated, and false if it
  // has not.
  bool isValid() const { return prod_ != 0; }

  ProductID id() const { return id_; }

  // Fill the given PtrVector<T> to refer to the same elements as
  // the View does. It is only safe to call this if isValid() is
  // true.
  void fill(PtrVector<T>& pv) const;

  // Conversion operators
  operator collection_type& ()             { return vals_; }
  operator collection_type const& () const { return vals_; }

  iterator begin() { return vals_.begin(); }
  iterator end()   { return vals_.end(); }
  const_iterator begin() const { return vals_.begin(); }
  const_iterator end() const   { return vals_.end(); }
  const_iterator cbegin() const { return vals_.cbegin(); }
  const_iterator cend() const   { return vals_.cend(); }

  size_type size() const { return vals_.size(); }

private:
  collection_type  vals_; // we do not own the pointed-to elements
  ProductID        id_;
  EDProduct const* prod_; // we do not own the product

  friend class Event;
  void set_innards(ProductID const& id, EDProduct const* p);
};

template <class T>
void
art::View<T>::fill(PtrVector<T>& pv) const
{
  std::vector<void const*> addresses;
  prod_->fillView(addresses);
  // Go through the pointers in vals_, look up each one in
  // addresses, and create the Ptr for each element.
  // TODO: optimize this loop
  typename collection_type::const_iterator vals_begin = vals_.begin();
  typename collection_type::const_iterator vals_end = vals_.end();
  for (size_t i = 0, sz = addresses.size(); i != sz; ++i)
    {
      if (std::find(vals_begin, vals_end, addresses[i]) != vals_end)
        {
          T const* p = reinterpret_cast<T const*>(addresses[i]);
          pv.push_back(Ptr<T>(id_,  const_cast<T*>(p), i));
        }
    }
}

template <class T>
inline
void
art::View<T>::set_innards(ProductID const& id,
                          EDProduct const* p)
{
  id_ = id;
  prod_ = p;
}

#endif /* art_Framework_Principal_View_h */

// Local Variables:
// mode: c++
// End:
