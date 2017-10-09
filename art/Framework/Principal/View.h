#ifndef art_Framework_Principal_View_h
#define art_Framework_Principal_View_h
// vim: set sw=2 expandtab :

// ====================================================================
// The class template View<T> provides a means to obtain pointers (of
// type T const*) into an arbitrary collection in an Event.
//
// A View<T> is *valid* if the container data product pointer is set.
// A valid View may still hold an empty vector of pointes; this means
// that either the referenced container was empty, or that the View's
// vector was emptied after the View was created.
//
// While View<T> is not a persistent class, one can fill a
// PtrVector<T> from a View<T>, as long as no new pointers have been
// added to the View<T>.
// ====================================================================

#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Common/PtrVector.h"
#include "cetlib/container_algorithms.h"

namespace art {

  template <class T>
  class View {

    // Give access to set_innards.
    friend class DataViewImpl;

  public:
    using collection_type = std::vector<T const*>;
    using value_type = typename collection_type::value_type;
    using const_iterator = typename collection_type::const_iterator;
    using iterator = typename collection_type::iterator;
    using size_type = typename collection_type::size_type;

  public:
    View() = default;

  public:
    collection_type&
    vals()
    {
      return vals_;
    }

    collection_type const&
    vals() const
    {
      return vals_;
    }

    // return true if this view has been populated, and false if it
    // has not.
    bool
    isValid() const
    {
      return prod_ != nullptr;
    }

    ProductID
    id() const
    {
      return id_;
    }

    // Fill the given PtrVector<T> to refer to the same elements as the
    // View does. It is only safe to call this if isValid() is true.
    // Fill a PtrVector<T> with Ptrs to each element
    // of the container data product that can be
    // reached from the internal vector<T const*>.
    void fill(PtrVector<T>& pv) const;

    // Conversion operators
    operator collection_type&() { return vals_; }

    operator collection_type const&() const { return vals_; }

    iterator
    begin()
    {
      return vals_.begin();
    }

    iterator
    end()
    {
      return vals_.end();
    }

    const_iterator
    begin() const
    {
      return vals_.begin();
    }

    const_iterator
    end() const
    {
      return vals_.end();
    }

    const_iterator
    cbegin() const
    {
      return vals_.cbegin();
    }

    const_iterator
    cend() const
    {
      return vals_.cend();
    }

    size_type
    size() const
    {
      return vals_.size();
    }

  private:
    void set_innards(ProductID const, EDProduct const*);

  private:
    // we do not own the pointed-to elements
    collection_type vals_{};

    ProductID id_{};

    // we do not own the product
    EDProduct const* prod_{nullptr};
  };

  // Fill a PtrVector<T> with Ptrs to each element
  // of the container data product that can be
  // reached from the internal vector<T const*>.
  template <class T>
  void
  View<T>::fill(PtrVector<T>& pv) const
  {
    std::vector<void const*> addresses;
    // Note: This calls Wrapper::fillView.
    prod_->fillView(addresses);
    std::size_t i{};
    for (auto a : addresses) {
      if (cet::search_all(vals_, a)) {
        auto p = reinterpret_cast<T const*>(a);
        pv.push_back(Ptr<T>{id_, const_cast<T*>(p), i});
        ++i;
      }
    }
  }

  template <class T>
  inline void
  View<T>::set_innards(ProductID const id, EDProduct const* p)
  {
    id_ = id;
    prod_ = p;
  }

} // namespace art

#endif /* art_Framework_Principal_View_h */

// Local Variables:
// mode: c++
// End:
