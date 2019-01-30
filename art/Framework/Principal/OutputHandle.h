#ifndef art_Framework_Principal_OutputHandle_h
#define art_Framework_Principal_OutputHandle_h
// vim: set sw=2 expandtab :

// Handle: Non-owning "smart pointer" for reference to EDProducts and
// their Provenances.
//
// This is a very preliminary version, and lacks safety features and
// elegance.
//
// If the pointed-to object or provenance destroyed, use of the
// Handle becomes undefined. There is no way to query the Handle to
// discover if this has happened.
//
// Handles can have:
//   -- Product and Provenance pointers both null;
//   -- Both pointers valid
//
// To check validity, one can use the isValid() function.
//
// If failedToGet() returns true then the requested data is not available
// If failedToGet() returns false but isValid() is also false then no attempt
//   to get data has occurred

#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib_except/exception.h"

#include <memory>

namespace art {

  class EDProduct;

  class OutputHandle {

  public: // MEMBER FUNCTIONS -- Special Member Functions
    ~OutputHandle();

    OutputHandle(EDProduct const*,
                 BranchDescription const*,
                 cet::exempt_ptr<ProductProvenance const>,
                 RangeSet const&);

    static OutputHandle
    invalid()
    {
      return OutputHandle{RangeSet::invalid()};
    }

    /// Used when the attempt to get the data failed
    OutputHandle(RangeSet const&);

    OutputHandle(OutputHandle const&) = delete;

    OutputHandle(OutputHandle&&) = default;

    OutputHandle& operator=(OutputHandle const&) = delete;

    OutputHandle& operator=(OutputHandle&&) = delete;

  public: // MEMBER FUNCTIONS -- API for the user
    bool isValid() const;

    BranchDescription const* desc() const;

    ProductProvenance const* productProvenance() const;

    EDProduct const* wrapper() const;

    RangeSet const& rangeOfValidity() const;

    void swap(OutputHandle&);

  private: // MEMBER DATA
    BranchDescription const* desc_{nullptr};

    cet::exempt_ptr<ProductProvenance const> productProvenance_{nullptr};

    EDProduct const* wrap_{nullptr};

    RangeSet const& rangeOfValidity_;
  };

  void swap(OutputHandle&, OutputHandle&);

} // namespace art

#endif /* art_Framework_Principal_OutputHandle_h */

// Local Variables:
// mode: c++
// End:
