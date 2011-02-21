#ifndef art_Framework_Core_View_h
#define art_Framework_Core_View_h

//
// The class template View<T> provides a means to obtain pointers (of
// type T const*) into an arbitrary collection in an Event.
//
// While View<T> is not a persistent class, one can fill a
// PtrVector<T> from a View<T>, as long as no new pointers have been
// added to the View<T>.
//

#include "art/Persistency/Common/PtrVector.h"

namespace art
{
  template <class T>
  class View
  {
  public:
    typedef std::vector<T const*> collection_type;

    View() : vals_(), id_(), prod_(0) { }
    collection_type&       vals()       { return vals_; }
    collection_type const& vals() const { return vals_; }

    // Fill the given PtrVector<T> to refer to the same elements as
    // the View does.
    void fill(PtrVector<T>& pv) const;

    // Conversion operators
    operator collection_type& ()             { return vals_; }
    operator collection_type const& () const { return vals_; }

  private:
    typedef T const*              value_type;

    collection_type  vals_;
    ProductID        id_;
    EDProduct const* prod_;

    friend class Event;
    void set_innards(ProductID const& id, EDProduct const* p);
  };

  template <class T>
  inline
  void
  View<T>::set_innards(ProductID const& id,
		       EDProduct const* p)
  {
    id_ = id;
    prod_ = p;
  }
}


#endif /* art_Framework_Core_View_h */

// Local Variables:
// mode: c++
// End:
