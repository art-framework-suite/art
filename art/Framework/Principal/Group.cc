#include "art/Framework/Principal/Group.h"

#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/OccurrenceTraits.h"
#include "art/Framework/Principal/Worker.h"
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
  productProducer_(),
  onDemandPrincipal_()
{ }

Group::Group(BranchDescription const& bd,
             ProductID const& pid,
             cet::exempt_ptr<Worker> productProducer,
             cet::exempt_ptr<EventPrincipal> onDemandPrincipal)
  :
  ppResolver_(),
  productResolver_(),
  product_(),
  branchDescription_(&bd),
  pid_(pid),
  productProducer_(productProducer),
  onDemandPrincipal_(onDemandPrincipal)
{ }

Group::Group(std::auto_ptr<EDProduct> edp,
             BranchDescription const& bd,
             ProductID const& pid) :
  ppResolver_(),
  productResolver_(),
  product_(edp.release()),
  branchDescription_(&bd),
  pid_(pid),
  productProducer_(),
  onDemandPrincipal_()
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
    productProducer_->
      doWork<OccurrenceTraits<EventPrincipal,
      BranchActionBegin> >(*onDemandPrincipal_, 0);
  } else {
    BranchKey const bk(productDescription());
    std::auto_ptr<EDProduct> edp(productResolver_->getProduct(bk));
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
  swap(productProducer_, other.productProducer_);
  swap(onDemandPrincipal_, other.onDemandPrincipal_);
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
