#include "art/Persistency/Provenance/ProductProvenance.h"

#include "art/Persistency/Provenance/ParentageRegistry.h"
#include <cassert>
#include <ostream>

namespace art {
  ProductProvenance::Transients::Transients() :
    parentagePtr_(),
    noParentage_(false)
  {}

  ProductProvenance::ProductProvenance() :
    branchID_(),
    productStatus_(productstatus::uninitialized()),
    parentageID_(),
    transients_()
  {}

  ProductProvenance::ProductProvenance(BranchID const& bid) :
    branchID_(bid),
    productStatus_(productstatus::uninitialized()),
    parentageID_(),
    transients_()
  {}

   ProductProvenance::ProductProvenance(BranchID const& bid,
                                        ProductStatus status) :
    branchID_(bid),
    productStatus_(status),
    parentageID_(),
    transients_()
  {}

   ProductProvenance::ProductProvenance(BranchID const& bid,
                                        ProductStatus status,
                                        ParentageID const& edid) :
    branchID_(bid),
    productStatus_(status),
    parentageID_(edid),
    transients_()
  {}

   ProductProvenance::ProductProvenance(BranchID const& bid,
                                    ProductStatus status,
                                    std::shared_ptr<Parentage> pPtr) :
    branchID_(bid),
    productStatus_(status),
    parentageID_(pPtr->id()),
    transients_() {
       parentagePtr() = pPtr;
       ParentageRegistry::put(*pPtr);
  }

  ProductProvenance::ProductProvenance(BranchID const& bid,
                   ProductStatus status,
                   std::vector<BranchID> const& parents) :
    branchID_(bid),
    productStatus_(status),
    parentageID_(),
    transients_() {
      parentagePtr() = std::shared_ptr<Parentage>(new Parentage);
      parentagePtr()->parents() = parents;
      parentageID_ = parentagePtr()->id();
      ParentageRegistry::put(*parentagePtr());
  }

  Parentage const &
  ProductProvenance::parentage() const {
    if (!parentagePtr()) {
      parentagePtr().reset(new Parentage);
      ParentageRegistry::get(parentageID_, *parentagePtr());
    }
    return *parentagePtr();
  }

  void
  ProductProvenance::setPresent() const {
    if (productstatus::present(productStatus())) return;
    assert(productstatus::unknown(productStatus()));
    setStatus(productstatus::present());
  }

  void
  ProductProvenance::setNotPresent() const {
    if (productstatus::neverCreated(productStatus())) return;
    if (productstatus::dropped(productStatus())) return;
    assert(productstatus::unknown(productStatus()));
    setStatus(productstatus::neverCreated());
  }

  void
  ProductProvenance::write(std::ostream& os) const {
    os << "branch ID = " << branchID() << '\n';
    os << "product status = " << static_cast<int>(productStatus()) << '\n';
    if (!noParentage()) {
      os << "entry description ID = " << parentageID() << '\n';
    }
  }

  // Only the 'salient attributes' are tested in equality comparison.
  bool
  operator==(ProductProvenance const& a, ProductProvenance const& b) {
    if (a.noParentage() != b.noParentage()) return false;
    if (a.noParentage()) {
      return
        a.branchID() == b.branchID()
        && a.productStatus() == b.productStatus();
    }
    return
      a.branchID() == b.branchID()
      && a.productStatus() == b.productStatus()
      && a.parentageID() == b.parentageID();
  }
}
