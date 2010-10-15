#ifndef DataFormats_Provenance_RunSubRunEntryInfo_h
#define DataFormats_Provenance_RunSubRunEntryInfo_h

/*----------------------------------------------------------------------

RunSubRunEntryInfo: The event dependent portion of the description of a product
and how it came into existence, plus the product identifier and the status.

----------------------------------------------------------------------*/
#include <iosfwd>
#include <vector>

#include "boost/shared_ptr.hpp"

#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/EntryDescriptionID.h"
#include "art/Persistency/Provenance/ModuleDescriptionID.h"
#include "art/Persistency/Provenance/ProductStatus.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "art/Persistency/Provenance/ProvenanceFwd.h"

/*
  RunSubRunEntryInfo
*/
namespace art {
  class RunSubRunEntryInfo {
  public:
    typedef std::vector<RunSubRunEntryInfo> EntryInfoVector;
    RunSubRunEntryInfo();
    explicit RunSubRunEntryInfo(BranchID const& bid);
    explicit RunSubRunEntryInfo(ProductProvenance const& ei);
    RunSubRunEntryInfo(BranchID const& bid,
		    ProductStatus status);
    RunSubRunEntryInfo(BranchID const& bid,
		    ProductStatus status,
		    ModuleDescriptionID const& mid,
		    std::vector<BranchID> const& parents = std::vector<BranchID>());

    RunSubRunEntryInfo(BranchID const& bid,
		    ProductStatus status,
		    EntryDescriptionID const& edid);

    ~RunSubRunEntryInfo() {}

    ProductProvenance makeProductProvenance() const;

    void write(std::ostream& os) const;

    BranchID const& branchID() const {return branchID_;}
    ProductStatus const& productStatus() const {return productStatus_;}
    ModuleDescriptionID const& moduleDescriptionID() const {return moduleDescriptionID_;}
    void setStatus(ProductStatus status) {productStatus_ = status;}
    void setModuleDescriptionID(ModuleDescriptionID const& mdid) {moduleDescriptionID_ = mdid;}
    void setPresent();
    void setNotPresent();
    ModuleDescriptionID const& entryDescriptionID() const {return moduleDescriptionID_;}

  private:
    BranchID branchID_;
    ProductStatus productStatus_;
    ModuleDescriptionID moduleDescriptionID_;
  };

  inline
  bool
  operator < (RunSubRunEntryInfo const& a, RunSubRunEntryInfo const& b) {
    return a.branchID() < b.branchID();
  }

  inline
  std::ostream&
  operator<<(std::ostream& os, RunSubRunEntryInfo const& p) {
    p.write(os);
    return os;
  }

  // Only the 'salient attributes' are testing in equality comparison.
  bool operator==(RunSubRunEntryInfo const& a, RunSubRunEntryInfo const& b);
  inline bool operator!=(RunSubRunEntryInfo const& a, RunSubRunEntryInfo const& b) { return !(a==b); }

  typedef RunSubRunEntryInfo SubRunEntryInfo;
  typedef RunSubRunEntryInfo RunEntryInfo;
}
#endif
