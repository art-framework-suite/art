#include "art/Framework/Core/Group.h"

#include "art/Persistency/Provenance/ProductStatus.h"
#include "art/Persistency/Provenance/ReflexTools.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include <string>

using art::Group;
using Reflex::Type;
using Reflex::TypeTemplate;

Group::Group() :
  product_(),
  branchDescription_(),
  pid_(),
  productProvenance_(),
  onDemand_(false)
{ }

Group::Group(BranchDescription const& bd, ProductID const& pid, bool demand) :
  product_(),
  branchDescription_(&bd),
  pid_(pid),
  productProvenance_(),
  onDemand_(demand)
{ }

Group::Group(std::auto_ptr<EDProduct> edp,
                  BranchDescription const& bd,
                  ProductID const& pid,
                  cet::exempt_ptr<ProductProvenance const> productProvenance) :
  product_(edp.release()),
  branchDescription_(&bd),
  pid_(pid),
  productProvenance_(productProvenance),
  onDemand_(false)
{ }

Group::Group(BranchDescription const& bd,
                  ProductID const& pid,
                  cet::exempt_ptr<ProductProvenance const> productProvenance) :
  product_(),
  branchDescription_(&bd),
  pid_(pid),
  productProvenance_(productProvenance),
  onDemand_(false)
{ }

art::ProductStatus
Group::status() const {
  if (dropped()) return productstatus::dropped();
  if (!productProvenance_) {
    if (product_) return product_->isPresent() ? productstatus::present() : productstatus::neverCreated();
    else return productstatus::unknown();
  }
  if (product_) {
    // for backward compatibility
    product_->isPresent() ? productProvenance_->setPresent() : productProvenance_->setNotPresent();
  }
  return productProvenance_->productStatus();
}

bool
Group::onDemand() const {
  return onDemand_;
}

bool
Group::productUnavailable() const {
  if (onDemand()) return false;
  if (dropped()) return true;
  if (productstatus::unknown(status())) return false;
  return not productstatus::present(status());

}

void
Group::setProduct(std::auto_ptr<EDProduct> prod) const {
  assert (product() == 0);
  product_.reset(prod.release());  // Group takes ownership
}

bool
Group::dropped() const {
  return branchDescription_ && (!branchDescription_->present());
}

void
Group::swap(Group& other) {
  using std::swap;
  swap(product_, other.product_);
  swap(branchDescription_, other.branchDescription_);
  swap(productProvenance_, other.productProvenance_);
  swap(onDemand_, other.onDemand_);
}

void
Group::replace(Group& g) {
  this->swap(g);
}

Type
Group::productType() const
{
  return Type::ByTypeInfo(typeid(*product()));
}

void
Group::write(std::ostream& os) const
{
  // This is grossly inadequate. It is also not critical for the
  // first pass.
  os << std::string("Group for product with ID: ")
     << pid_;
}

void
Group::mergeGroup(Group * newGroup) {

  if (status() != newGroup->status()) {
    throw art::Exception(art::errors::Unknown, "Merging")
      << "Group::mergeGroup(), Trying to merge two run products or two subRun products.\n"
      << "The products have different creation status's.\n"
      << "For example \"present\" and \"notCreated\"\n"
      << "The Framework does not currently support this and probably\n"
      << "should not support this case.\n"
      << "Likely a problem exists in the producer that created these\n"
      << "products.  If this problem cannot be reasonably resolved by\n"
      << "fixing the producer module, then contact the Framework development\n"
      << "group with details so we can discuss whether and how to support this\n"
      << "use case.\n"
      << "className = " << branchDescription_->className() << "\n"
      << "moduleLabel = " << moduleLabel() << "\n"
      << "instance = " << productInstanceName() << "\n"
      << "process = " << processName() << "\n";
  }

  if (!productProvenance_) {
    return;
  }

  if (!productUnavailable() && !newGroup->productUnavailable()) {

    if (product_->isMergeable()) {
      product_->mergeProduct(newGroup->product_.get());
    }
    else if (product_->hasIsProductEqual()) {

      if (!product_->isProductEqual(newGroup->product_.get())) {
        mf::LogWarning("RunSubRunMerging")
          << "Group::mergeGroup\n"
          "Two run/subRun products for the same run/subRun which should be equal are not\n"
          "Using the first, ignoring the second\n"
          << "className = " << branchDescription_->className() << "\n"
          << "moduleLabel = " << moduleLabel() << "\n"
          << "instance = " << productInstanceName() << "\n"
          << "process = " << processName() << "\n";
      }
    }
    else {
      mf::LogWarning("RunSubRunMerging")
        << "Group::mergeGroup\n"
        "Run/subRun product has neither a mergeProduct nor isProductEqual function\n"
        "Using the first, ignoring the second in merge\n"
        << "className = " << branchDescription_->className() << "\n"
        << "moduleLabel = " << moduleLabel() << "\n"
        << "instance = " << productInstanceName() << "\n"
        << "process = " << processName() << "\n";
    }
  }
}

void
Group::resolveProvenance(BranchMapper const &mapper) const
{
  if (!productProvenancePtr())
    productProvenance_ = mapper.branchToProductProvenance(productDescription().branchID());
}

// ======================================================================
