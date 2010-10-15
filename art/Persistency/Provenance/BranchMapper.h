#ifndef DataFormats_Provenance_BranchMapper_h
#define DataFormats_Provenance_BranchMapper_h

/*----------------------------------------------------------------------

BranchMapper: Manages the per event/subRun/run per product provenance.

----------------------------------------------------------------------*/
#include <iosfwd>
#include <set>
#include <map>
#include "boost/shared_ptr.hpp"

#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "art/Utilities/Algorithms.h"

/*
  BranchMapper

*/

namespace art {
  class ProductID;
  class BranchMapper {
  public:
    BranchMapper();

    explicit BranchMapper(bool delayedRead);

    virtual ~BranchMapper() {}

    void write(std::ostream& os) const;

    boost::shared_ptr<ProductProvenance> branchToEntryInfo(BranchID const& bid) const;

    void insert(ProductProvenance const& provenanceProduct);

    void mergeMappers(boost::shared_ptr<BranchMapper> other) {nextMapper_ = other;}

    void setDelayedRead(bool value) {delayedRead_ = value;}

    BranchID oldProductIDToBranchID(ProductID const& oldProductID) const {
      return oldProductIDToBranchID_(oldProductID);
    }

  private:
    typedef std::set<ProductProvenance> eiSet;

    void readProvenance() const;
    virtual void readProvenance_() const {}

    virtual BranchID oldProductIDToBranchID_(ProductID const& oldProductID) const;

    eiSet entryInfoSet_;

    boost::shared_ptr<BranchMapper> nextMapper_;

    mutable bool delayedRead_;

  };

  inline
  std::ostream&
  operator<<(std::ostream& os, BranchMapper const& p) {
    p.write(os);
    return os;
  }
}
#endif
