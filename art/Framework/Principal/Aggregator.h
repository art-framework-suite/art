#ifndef art_Framework_Principal_Aggregator_h
#define art_Framework_Principal_Aggregator_h

#include "art/Framework/Principal/Handle.h"
#include "canvas/Persistency/Common/detail/aggregate.h"
#include "canvas/Persistency/Provenance/RangeSet.h"

#include <memory>

namespace art {

  template <typename T>
  class Aggregator {
  public:

    static_assert(detail::CanBeAggregated<T>::value,
                  "\n\n"
                  "art error: Aggregator<T>'s support only products that can be aggregated.\n"
                  "           Please contact artists@fnal.gov.\n");

    template <typename U>
    std::enable_if_t<detail::is_handle<U>::value>
    update(U const& h);

    template <typename U>
    std::enable_if_t<detail::is_handle<U>::value>
    update(U const& h, T const& t);

    bool isValid() const;

    T const& product() const;
    RangeSet const& rangeSet() const;

  private:
    bool uninitialized() const;

    template <typename U>
    void reset(U const& h, T t);

    template <typename U>
    bool need_new_aggregator(U const& h);

    T product_ {};
    RangeSet rangeSet_ {RangeSet::invalid()};
  };

  //===============================================
  // Implementation

  template <typename T>
  template <typename U>
  void
  Aggregator<T>::reset(U const& h, T t)
  {
    auto newRS = h.provenance()->rangeSet();
    std::swap(product_, t);
    std::swap(rangeSet_, newRS);
  }

  template <typename T>
  template <typename U>
  bool
  Aggregator<T>::need_new_aggregator(U const& h)
  {
    auto const& newRS = h.provenance()->rangeSet();
    switch (h.provenance()->productDescription().branchType()) {
    case InSubRun : {
      if (rangeSet_.empty()) return true;
      return newRS.ranges().front().subrun() != rangeSet_.ranges().front().subrun();
    }
    case InRun:
      return newRS.run() != rangeSet_.run();
    default:
      throw art::Exception{errors::ProductCannotBeAggregated, "Aggregator<T>::new_aggregator"}
         << "An attempt to aggregate has been made on a product\n"
         << "that is not a SubRun or Run product.\n";
    }
    return true; // Will never get here
  }

  template <typename T>
  template <typename U>
  std::enable_if_t<detail::is_handle<U>::value>
  Aggregator<T>::update(U const& h, T const& t)
  {
    if (need_new_aggregator(h))
      reset(h,t);

    if (!isValid())
      throw art::Exception(errors::LogicError, "Aggregator<T>::update")
        << "Ill-formed Aggregator instance--either the product is empty,\n"
        << "or the RangeSet is empty.  Contact artists@fnal.gov\n";

    if (art::disjoint_ranges(rangeSet_, h.provenance()->rangeSet())) {
      detail::CanBeAggregated<T>::aggregate(product_, t);
      rangeSet_.merge(h.provenance()->rangeSet());
    }
  }


  template <typename T>
  template <typename U>
  std::enable_if_t<detail::is_handle<U>::value>
  Aggregator<T>::update(U const& h)
  {
    update(h, *h);
  }

  template <typename T>
  inline
  bool
  Aggregator<T>::isValid() const
  {
    return rangeSet_.is_valid();
  }

  template <typename T>
  inline
  T const&
  Aggregator<T>::product() const
  {
    return product_;
  }

  template <typename T>
  RangeSet const&
  Aggregator<T>::rangeSet() const
  {
    return rangeSet_;
  }


  template <class T>
  RangeSet const&
  range_set(Aggregator<T> const& a)
  {
    std::string const& errMsg = "Attempt to retrieve range set from invalid Aggregator.";
    detail::throw_if_invalid(errMsg, a);
    return a.rangeSet();
  }

  template <class T, class U>
  bool
  same_ranges(Aggregator<T> const& a, Aggregator<U> const& b)
  {
    std::string const& errMsg = "Attempt to compare range sets where one or both Aggregators are invalid.";
    detail::throw_if_invalid(errMsg, a,b);
    return same_ranges(a.rangeSet(), b.rangeSet());
  }

}

#endif

// Local variables:
// mode: c++
// End:
