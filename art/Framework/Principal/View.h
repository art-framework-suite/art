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

#include <string>
#include <utility>

namespace art {

  class DataViewImpl;
  class InputTag;

  template <typename T>
  class View {

    // Give access to private ctor.
    template <typename Element>
    friend bool DataViewImpl::getView(std::string const&,
                                      std::string const&,
                                      std::string const&,
                                      View<Element>&) const;

    // TYPES
  public:
    using collection_type = std::vector<T const*>;
    using value_type = typename collection_type::value_type;
    using const_iterator = typename collection_type::const_iterator;
    using iterator = typename collection_type::iterator;
    using size_type = typename collection_type::size_type;

    // MEMBER FUCTIONS -- Special Member Functions
  public:
    View() = default;

    // MEMBER FUCTIONS -- Special Member Functions
  private:
    View(std::vector<T const*>&, ProductID const, EDProduct const*);

    // MEMBER FUNCTIONS -- API for user
  public:
    auto
    isValid() const
    {
      return prod_ != nullptr;
    }

    auto
    id() const
    {
      return id_;
    }

    // Fill a PtrVector<T> with Ptrs to each element of the container data
    // product that can be reached from the internal vector<T const*>.
    void fill(PtrVector<T>& pv) const;

    auto
    vals() -> auto&
    {
      return vals_;
    }

    auto
    vals() const -> auto const&
    {
      return vals_;
    }

    operator auto&() { return vals_; }

    operator auto const&() const { return vals_; }

    auto
    begin()
    {
      return vals_.begin();
    }

    auto
    end()
    {
      return vals_.end();
    }

    auto
    begin() const -> decltype(std::vector<T const*>{}.cbegin())
    {
      return vals_.begin();
    }

    auto
    end() const -> decltype(std::vector<T const*>{}.cend())
    {
      return vals_.end();
    }

    auto
    cbegin() const
    {
      return vals_.cbegin();
    }

    auto
    cend() const
    {
      return vals_.cend();
    }

    auto
    size() const
    {
      return vals_.size();
    }

    // MEMBER DATA -- Implementation details
  private:
    // Vector of pointers to elements in the container product (this is the view
    // itself).
    std::vector<T const*> vals_{};

    // The container id.
    ProductID id_{};

    // The container itself, we do not own.
    EDProduct const* prod_{nullptr};
  };

  template <typename T>
  View<T>::View(std::vector<T const*>& v,
                ProductID const id,
                EDProduct const* p)
    : vals_{std::move(v)}, id_{id}, prod_{p}
  {}

  // Fill a PtrVector<T> with Ptrs to each element
  // of the container data product that can be
  // reached from the internal vector<T const*>.
  template <typename T>
  void
  View<T>::fill(PtrVector<T>& pv) const
  {
    std::vector<void const*> view;
    // Note: This calls Wrapper::fillView.
    prod_->fillView(view);
    typename std::vector<T const*>::size_type i{};
    for (auto a : view) {
      // We do this as a sloppy guard against the container
      // data product having been replaced between the
      // View<T> ctor and the fill call. The user is
      // supposed to do event.getView<T>() immediately
      // followed by View<T>::fill(), but well. This can be
      // O(n^2) so maybe it is not worth it in the case of
      // large containers.
      if (std::find(vals_.cbegin(), vals_.cend(), a) != vals_.cend()) {
        auto p = reinterpret_cast<T const*>(a);
        pv.push_back(Ptr<T>{id_, const_cast<T*>(p), i});
        ++i;
      }
    }
  }

} // namespace art

#endif /* art_Framework_Principal_View_h */

// Local Variables:
// mode: c++
// End:
