#ifndef art_Framework_Principal_SummedValue_h
#define art_Framework_Principal_SummedValue_h

// ======================================================================
//
// SummedValue: Class whose instances own two objects:
//              - object of type specified as the template argument
//              - RangeSet corresponding to the object
//
// The purpose of this auxiliary class is to provide users with a
// means of tying an object with its associated range.  This can be
// important whenever a user needs to assemble (e.g.) product 3 from
// products 1 and 2.  However, even though products 1 and 2 might have
// the same RangeSets overall, due to output-file switching, the
// RangeSets as recorded in each input file may be different.  In such
// a case, product 3 should arguably only be placed on the Run or
// SubRun whenever the RangeSets from products 1 and 2 are the same.
//
// The main function of interest is SummedValue::update(Handle const& h),
// which updates the owned object (calling its appropriate aggregation
// function) and its corresponding RangeSet.
//
// The type 'T' need not correspond to the type of the data product.
// For example, the following are allowed uses of SummedValue:
//
//   class MyModule : public EDProducer {
//     SummedValue<POTSummary> pots_;
//     SummedValue<unsigned> count_;
//   public:
//     ...
//     void produce(Run& r) {
//       auto const& h = r.getValidHandle<POTSummary>(...);
//       pots_.update(h);
//       count_.update(h, h->proton_count()); // Ties 'count_' with RangeSet
//       from 'h'
//     }
//   };
//
// N.B. It is the responsibility of the user to call 'clear' whenever
//      the owned object has been fully updated for the Run or SubRun
//      of interest.
// ======================================================================

#include "art/Framework/Principal/Handle.h"
#include "canvas/Persistency/Common/detail/aggregate.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "cetlib_except/demangle.h"

#include <memory>

namespace art {

  template <typename T>
  class SummedValue {
  public:
    static_assert(
      detail::CanBeAggregated<T>::value,
      "\n\n"
      "art error: SummedValue<T>'s only support types that can be aggregated.\n"
      "           Please contact artists@fnal.gov.\n");

    template <typename H>
    std::enable_if_t<detail::is_handle<H>::value> update(H const& h);

    template <typename H>
    std::enable_if_t<detail::is_handle<H>::value> update(H const& h,
                                                         T const& t);

    void clear();

    // Default-constructed object is invalid.  As soon as it is
    // updated, it becomes valid.  No update can invalidate it.
    bool isValid() const;

    T const& value() const;
    RangeSet const& rangeOfValidity() const;

  private:
    template <typename H>
    void
    update_impl(H const& h, T const& t)
    {
      // Precondition: handle must be valid
      assert(h.isValid());

      auto const& newRS = h.provenance()->rangeOfValidity();
      if (!rangeOfValidity_.is_valid() && newRS.is_valid()) {
        rangeOfValidity_ = newRS;
        value_ = t;
      } else if (art::disjoint_ranges(rangeOfValidity_, newRS)) {
        detail::CanBeAggregated<T>::aggregate(value_, t);
        rangeOfValidity_.merge(h.provenance()->rangeOfValidity());
      } else if (art::same_ranges(rangeOfValidity_, newRS)) {
        // The ranges are the same, so the behavior is a NOP.
        // However, we will probably never get here because of the
        // seenIDs set, which prevents from duplicate aggregation.
        // If the stakeholders decide that products with the same
        // ranges should be checked for equality, then the seenIDs
        // set needs to go away, and an extra condition will be
        // added here.
      } else if (art::overlapping_ranges(rangeOfValidity_, newRS)) {
        throw Exception{errors::ProductCannotBeAggregated,
                        "SummedValue<T>::update"}
          << "\nThe following ranges corresponding to the type:\n"
          << "   '" << cet::demangle_symbol(typeid(T).name()) << "'"
          << "\ncannot be aggregated\n"
          << rangeOfValidity_ << " and\n"
          << newRS << "\nPlease contact artists@fnal.gov.\n";
      }
      // NOP when both RangeSets are invalid
    }

    T value_{};
    RangeSet rangeOfValidity_{RangeSet::invalid()};
  };

  //===============================================
  // Implementation

  template <typename T>
  template <typename H>
  std::enable_if_t<detail::is_handle<H>::value>
  SummedValue<T>::update(H const& h)
  {
    std::string const& errMsg{"Attempt to update " +
                              cet::demangle_symbol(typeid(*this).name()) +
                              " from an invalid handle."};
    detail::throw_if_invalid(errMsg, h);
    update_impl(h, *h);
  }

  template <typename T>
  template <typename H>
  std::enable_if_t<detail::is_handle<H>::value>
  SummedValue<T>::update(H const& h, T const& t)
  {
    std::string const& errMsg{"Attempt to update " +
                              cet::demangle_symbol(typeid(*this).name()) +
                              " from an invalid handle.\n"};
    detail::throw_if_invalid(errMsg, h);
    update_impl(h, t);
  }

  template <typename T>
  inline void
  SummedValue<T>::clear()
  {
    SummedValue<T> tmp{};
    std::swap(*this, tmp);
  }

  template <typename T>
  inline bool
  SummedValue<T>::isValid() const
  {
    return rangeOfValidity_.is_valid();
  }

  template <typename T>
  inline T const&
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

  template <class T, class U>
  bool
  same_ranges(SummedValue<T> const& a, SummedValue<U> const& b)
  {
    return same_ranges(a.rangeOfValidity(), b.rangeOfValidity());
  }

  template <class T, class U>
  bool
  disjoint_ranges(SummedValue<T> const& a, SummedValue<U> const& b)
  {
    return disjoint_ranges(a.rangeOfValidity(), b.rangeOfValidity());
  }

  template <class T, class U>
  bool
  overlapping_ranges(SummedValue<T> const& a, SummedValue<U> const& b)
  {
    return overlapping_ranges(a.rangeOfValidity(), b.rangeOfValidity());
  }
}

#endif /* art_Framework_Principal_SummedValue_h */

// Local variables:
// mode: c++
// End:
