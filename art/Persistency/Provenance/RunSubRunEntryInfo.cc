#include "art/Persistency/Provenance/ProductProvenance.h"
#include "art/Persistency/Provenance/RunSubRunEntryInfo.h"
#include "art/Persistency/Provenance/EntryDescriptionID.h"
#include "art/Persistency/Provenance/EventEntryDescription.h"
#include "art/Persistency/Provenance/EntryDescriptionRegistry.h"
#include <ostream>

/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

namespace art {
  RunSubRunEntryInfo::RunSubRunEntryInfo() :
    branchID_(),
    productStatus_(productstatus::uninitialized()),
    moduleDescriptionID_()
  {}

  RunSubRunEntryInfo::RunSubRunEntryInfo(ProductProvenance const& ei) :
    branchID_(ei.branchID()),
    productStatus_(ei.productStatus()),
    moduleDescriptionID_()
  {}

  RunSubRunEntryInfo::RunSubRunEntryInfo(BranchID const& bid) :
    branchID_(bid),
    productStatus_(productstatus::uninitialized()),
    moduleDescriptionID_()
  {}

   RunSubRunEntryInfo::RunSubRunEntryInfo(BranchID const& bid,
				    ProductStatus status) :
    branchID_(bid),
    productStatus_(status),
    moduleDescriptionID_()
  {}

   // The last two arguments are ignored.
   // They are used for backward compatibility.
   RunSubRunEntryInfo::RunSubRunEntryInfo(BranchID const& bid,
				    ProductStatus status,
				    ModuleDescriptionID const& mid,
				    std::vector<BranchID> const&) :
    branchID_(bid),
    productStatus_(status),
    moduleDescriptionID_(mid)
  {}

   RunSubRunEntryInfo::RunSubRunEntryInfo(BranchID const& bid,
				    ProductStatus status,
				    EntryDescriptionID const& edid) :
    branchID_(bid),
    productStatus_(status),
    moduleDescriptionID_() {
     EventEntryDescription ed;
     EntryDescriptionRegistry::instance()->getMapped(edid, ed);
     moduleDescriptionID_ = ed.moduleDescriptionID();
  }

  ProductProvenance
  RunSubRunEntryInfo::makeProductProvenance() const {
    return ProductProvenance(branchID_, productStatus_);
  }

  void
  RunSubRunEntryInfo::setPresent() {
    if (productstatus::present(productStatus())) return;
    assert(productstatus::unknown(productStatus()));
    setStatus(productstatus::present());
  }

  void
  RunSubRunEntryInfo::setNotPresent() {
    if (productstatus::neverCreated(productStatus())) return;
    assert(productstatus::unknown(productStatus()));
    setStatus(productstatus::neverCreated());
  }

  void
  RunSubRunEntryInfo::write(std::ostream& os) const {
    os << "branch ID = " << branchID() << '\n';
    os << "product status = " << productStatus() << '\n';
    os << "module description ID = " << moduleDescriptionID() << '\n';
  }

  bool
  operator==(RunSubRunEntryInfo const& a, RunSubRunEntryInfo const& b) {
    return
      a.branchID() == b.branchID()
      && a.productStatus() == b.productStatus()
      && a.moduleDescriptionID() == b.moduleDescriptionID();
  }
}
