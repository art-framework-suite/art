#include "art/Persistency/Common/Group.h"

#include "art/Persistency/Provenance/BranchKey.h"
#include "art/Persistency/Provenance/ProductStatus.h"
#include "art/Persistency/Provenance/ReflexTools.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include <string>

using art::Group;
using Reflex::Type;
using Reflex::TypeTemplate;

Group::Group() :
  ppResolver_(),
  productResolver_(),
  product_(),
  branchDescription_(),
  pid_(),
  onDemand_(false)
{ }

Group::Group(BranchDescription const& bd,
             ProductID const& pid,
             bool demand) :
  ppResolver_(),
  productResolver_(),
  product_(),
  branchDescription_(&bd),
  pid_(pid),
  onDemand_(demand)
{ }

Group::Group(std::auto_ptr<EDProduct> edp,
             BranchDescription const& bd,
             ProductID const& pid) :
  ppResolver_(),
  productResolver_(),
  product_(edp.release()),
  branchDescription_(&bd),
  pid_(pid),
  onDemand_(false)
{ }

Group::~Group()
{ }

art::ProductStatus
Group::status() const {
  if (dropped()) return productstatus::dropped();
  cet::exempt_ptr<ProductProvenance const> pp(productProvenancePtr());
  if (!pp) {
    if (product_) {
      if (product_->isPresent()) {
        return productstatus::present();
      } else {
        return productstatus::neverCreated();
      }
    } else {
      return productstatus::unknown();
    }
  }
  if (product_) { // FIXME: Old CMS note said backward compatibility only?
    if (product_->isPresent()) {
      pp->setPresent();
    } else {
      pp->setNotPresent();
    }
  }
  return pp->productStatus();
}

bool
Group::resolveProduct(bool fillOnDemand) const {
  if (productUnavailable()) {
    throw art::Exception(errors::ProductNotFound,"InaccessibleProduct")
      << "resolveProduct: product is not accessible\n"
      << productDescription() << '\n'
      << *productProvenancePtr() << '\n';
  }
  return resolveProductIfAvailable(fillOnDemand);
}

bool
Group::resolveProductIfAvailable(bool fillOnDemand) const {
  if (product()) return true; // Nothing to do.
  if (productUnavailable()) return false; // Nothing we *can* do.

  // Try unscheduled production.
  if (fillOnDemand && onDemand()) {
#if UNSCHEDULED_ENABLED // re-enable once on-demand logic is available to Group
    unscheduledFill(productDescription().moduleLabel());
#endif  // 0
  } else {
    BranchKey const bk(productDescription());
    std::auto_ptr<EDProduct> edp(productResolver_->getProduct(bk, this));
    // Now fix up the Group
    setProduct(edp);
  }
  return product();
}

bool
Group::productUnavailable() const {
  if (onDemand()) return false;
  if (dropped()) return true;
  if (productstatus::unknown(status())) return false;
  return not productstatus::present(status());
}

cet::exempt_ptr<art::ProductProvenance const>
Group::productProvenancePtr() const {
  if (ppResolver_) {
    return ppResolver_->branchToProductProvenance(branchDescription_->branchID());
  } else {
    return cet::exempt_ptr<ProductProvenance const>();
  }
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
  swap(ppResolver_, other.ppResolver_);
  swap(product_, other.product_);
  swap(branchDescription_, other.branchDescription_);
  swap(pid_, other.pid_);
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
  os << "Group for product with ID: " << pid_;
}

void
Group::mergeGroup(Group * newGroup) {

  if (status() != newGroup->status()) {
    throw art::Exception(art::errors::Unknown, "Merging")
      << "Group::mergeGroup(), Trying to merge two run products or two subRun products.\n"
         "The products have different creation status's.\n"
         "For example \"present\" and \"notCreated\"\n"
         "The Framework does not currently support this and probably\n"
         "should not support this case.\n"
         "Likely a problem exists in the producer that created these\n"
         "products.  If this problem cannot be reasonably resolved by\n"
         "fixing the producer module, then contact the Framework development\n"
         "group with details so we can discuss whether and how to support this\n"
         "use case.\n"
         "className = " << branchDescription_->className() << "\n"
         "moduleLabel = " << moduleLabel() << "\n"
         "instance = " << productInstanceName() << "\n"
         "process = " << processName() << "\n";
  }

  if (!productProvenancePtr()) {
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
             "className = " << branchDescription_->className() << "\n"
             "moduleLabel = " << moduleLabel() << "\n"
             "instance = " << productInstanceName() << "\n"
             "process = " << processName() << "\n";
      }
    }
    else {
      mf::LogWarning("RunSubRunMerging")
        << "Group::mergeGroup\n"
           "Run/subRun product has neither a mergeProduct nor isProductEqual function\n"
           "Using the first, ignoring the second in merge\n"
           "className = " << branchDescription_->className() << "\n"
           "moduleLabel = " << moduleLabel() << "\n"
           "instance = " << productInstanceName() << "\n"
           "process = " << processName() << "\n";
    }
  }
}
