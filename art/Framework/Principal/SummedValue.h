#ifndef art_Framework_Principal_SummedValue_h
#define art_Framework_Principal_SummedValue_h

#include "art/Framework/Principal/Handle.h"
#include "canvas/Persistency/Common/detail/aggregate.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "cetlib/demangle.h"

#include <memory>

namespace art {

  template <typename T>
  class SummedValue {
  public:

    static_assert(detail::CanBeAggregated<T>::value,
                  "\n\n"
                  "art error: SummedValue<T>'s only support types that can be aggregated.\n"
                  "           Please contact artists@fnal.gov.\n");

    template <typename U>
    std::enable_if_t<detail::is_handle<U>::value>
    update(U const& h);

    template <typename U>
    std::enable_if_t<detail::is_handle<U>::value>
    update(U const& h, T const& t);

    // Default-c'tored object is invalid.  As soon as it is updated,
    // it becomes valid.  No update can invalidate it.
    bool isValid() const;

    T const& value() const;
    RangeSet const& rangeOfValidity() const;

  private:
    bool uninitialized() const;

    template <typename U>
    void reset(U const& h, T t);

    template <typename U>
    bool should_reset(U const& h);

    T value_ {};
    RangeSet rangeOfValidity_ {RangeSet::invalid()};
  };

  //===============================================
  // Implementation

  template <typename T>
  template <typename U>
  void
  SummedValue<T>::reset(U const& h, T t)
  {
    auto newRS = h.provenance()->rangeOfValidity();
    std::swap(value_, t);
    std::swap(rangeOfValidity_, newRS);
  }

  template <typename T>
  template <typename U>
  bool
  SummedValue<T>::should_reset(U const& h)
  {
    auto const& newRS = h.provenance()->rangeOfValidity();
    switch (h.provenance()->productDescription().branchType()) {
    case InSubRun : {
      if (rangeOfValidity_.empty()) return true;
      return newRS.ranges().front().subRun() != rangeOfValidity_.ranges().front().subRun();
    }
    case InRun:
      return newRS.run() != rangeOfValidity_.run();
    default:
      throw art::Exception{errors::ProductCannotBeAggregated, "SummedValue<T>::new_aggregator"}
         << "An attempt to aggregate has been made on a product\n"
         << "that is not a SubRun or Run product.\n";
    }
    return true; // Will never get here
  }

  template <typename T>
  template <typename U>
  std::enable_if_t<detail::is_handle<U>::value>
  SummedValue<T>::update(U const& h, T const& t)
  {
    if (should_reset(h))
      reset(h,t);

    if (!isValid())
      throw art::Exception(errors::LogicError, "SummedValue<T>::update")
        << "The range-of-validity is invalid for the object:"
        << cet::demangle(typeid(*this).name()) << '\n'
        << "Please contact artists@fnal.gov\n";

    if (art::disjoint_ranges(rangeOfValidity_, h.provenance()->rangeOfValidity())) {
      detail::CanBeAggregated<T>::aggregate(value_, t);
      rangeOfValidity_.merge(h.provenance()->rangeOfValidity());
    }
    // else if (art::same_ranges(...) && value_ != *h)
    // else if (art::overlapping_ranges(...))
    //   throw...;
  }


  template <typename T>
  template <typename U>
  std::enable_if_t<detail::is_handle<U>::value>
  SummedValue<T>::update(U const& h)
  {
    update(h, *h);
  }

  template <typename T>
  inline
  bool
  SummedValue<T>::isValid() const
  {
    return rangeOfValidity_.is_valid();
  }

  template <typename T>
  inline
  T const&
  SummedValue<T>::value() const
  {
    return value_;
  }

  template <typename T>
  RangeSet const&
  SummedValue<T>::rangeOfValidity() const
  {
    return rangeOfValidity_;
  }


  template <class T>
  RangeSet const&
  range_of_validity(SummedValue<T> const& a)
  {
    std::string const& errMsg = "Attempt to retrieve range-of-validity from invalid SummedValue.";
    detail::throw_if_invalid(errMsg, a);
    return a.rangeOfValidity();
  }

  template <class T, class U>
  bool
  same_ranges(SummedValue<T> const& a, SummedValue<U> const& b)
  {
    std::string const& errMsg = "Attempt to compare range sets where one or both SummedValues are invalid.";
    detail::throw_if_invalid(errMsg, a,b);
    return same_ranges(a.rangeOfValidity(), b.rangeOfValidity());
  }

}

#endif

// Local variables:
// mode: c++
// End:
