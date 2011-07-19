#ifndef art_Persistency_Provenance_BranchMapper_h
#define art_Persistency_Provenance_BranchMapper_h

// ======================================================================
//
// BranchMapper: Manages the per event/subRun/run per product provenance.
//
// ======================================================================

#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "cpp0x/memory"
#include "cetlib/container_algorithms.h"
#include <iosfwd>
#include <map>
#include <set>

// ----------------------------------------------------------------------

namespace art {

  class ProductID;

  class BranchMapper {
  public:
    BranchMapper();

    explicit BranchMapper(bool delayedRead);

    virtual ~BranchMapper() {}

    void write(std::ostream& os) const;

    std::shared_ptr<ProductProvenance> branchToProductProvenance(BranchID const& bid) const;

    void insert(ProductProvenance const& provenanceProduct);

    void setDelayedRead(bool value) {delayedRead_ = value;}

  private:
    typedef std::map<BranchID, ProductProvenance> eiSet;

    void readProvenance() const;
    virtual void readProvenance_() const {}

    eiSet entryInfoSet_;

    mutable bool delayedRead_;

  };

  inline
  std::ostream&
  operator<<(std::ostream& os, BranchMapper const& p) {
    p.write(os);
    return os;
  }

}  // art

// ======================================================================

#endif /* art_Persistency_Provenance_BranchMapper_h */

// Local Variables:
// mode: c++
// End:
