#include "art/Framework/Principal/Group.h"
// vim: set sw=2:

#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/OccurrenceTraits.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Persistency/Provenance/BranchKey.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Persistency/Provenance/ProductStatus.h"
#include "art/Persistency/Provenance/TypeTools.h"
#include "cetlib/demangle.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <iostream>
#include <string>

namespace art {

Group::
Group()
  : wrapper_type_()
  , ppResolver_()
  , productResolver_()
  , product_()
  , branchDescription_()
  , pid_()
  , productProducer_()
  , onDemandPrincipal_()
{
}

Group::
Group(BranchDescription const& bd, ProductID const& pid,
      art::TypeID const& wrapper_type, cet::exempt_ptr<Worker> productProducer,
      cet::exempt_ptr<EventPrincipal> onDemandPrincipal)
  : wrapper_type_(wrapper_type)
  , ppResolver_()
  , productResolver_()
  , product_()
  , branchDescription_(&bd)
  , pid_(pid)
  , productProducer_(productProducer)
  , onDemandPrincipal_(onDemandPrincipal)
{
}

Group::
Group(std::unique_ptr<EDProduct>&& edp, BranchDescription const& bd,
      ProductID const& pid, art::TypeID const& wrapper_type)
  : wrapper_type_(wrapper_type)
  , ppResolver_()
  , productResolver_()
  , product_(std::move(edp))
  , branchDescription_(&bd)
  , pid_(pid)
  , productProducer_()
  , onDemandPrincipal_()
{
}

art::ProductStatus
Group::
status() const
{
  if (dropped()) {
    return productstatus::dropped();
  }
  cet::exempt_ptr<ProductProvenance const> pp(productProvenancePtr());
  if (!pp) {
    // No provenance, must be new.
    if (product_.get()) {
      if (product_->isPresent()) {
        return productstatus::present();
      }
      return productstatus::neverCreated();
    }
    return productstatus::unknown();
  }
  if (product_.get()) {
    // Product has already been delay read, use the present flag
    // from the wrapper.
    // FIXME: Old CMS note said backward compatibility only?
    if (product_->isPresent()) {
      pp->setPresent();
    }
    else {
      pp->setNotPresent();
    }
  }
  // Not new, and not yet read, use the status from the provenance.
  return pp->productStatus();
}

bool
Group::
resolveProduct(bool fillOnDemand, TypeID const& wanted_wrapper_type) const
{
  if (!productUnavailable()) {
    return resolveProductIfAvailable(fillOnDemand, wanted_wrapper_type);
  }
  art::Exception e(errors::ProductNotFound, "InaccessibleProduct");
  e << "resolveProduct: product is not accessible\n"
    << productDescription()
    << '\n';
  if (productProvenancePtr()) {
    e << *productProvenancePtr() << '\n';
  }
  throw e;
}

bool
Group::
resolveProductIfAvailable(bool fillOnDemand,
                          TypeID const& wanted_wrapper_type) const
{
  if (product_.get()) {
    // Already resolved.
    return true;
  }
  if (productUnavailable()) {
    // It is possible for a product to be on the product list but the
    // run product is not present.
    return false;
  }

  if (wanted_wrapper_type != wrapper_type_) {
    throw Exception(errors::LogicError)
        << "Attempted to obtain a product of different type ("
        << wanted_wrapper_type.className()
        << ") than produced ("
        << wrapper_type_.className()
        << ").\n";
  }
  std::unique_ptr<EDProduct>
  edp(obtainDesiredProduct(fillOnDemand, wanted_wrapper_type));
  if (edp.get()) {
    setProduct(std::move(edp));
  }
  return product_.get();
}

std::unique_ptr<art::EDProduct>
Group::
obtainDesiredProduct(bool fillOnDemand, TypeID const& wanted_wrapper_type) const
{
  //std::cout
  //    << "-----> Begin Group::obtainDesiredProduct(bool, TypeID const&)"
  //    << std::endl;
  //std::cout
  //    << "wt: "
  //    << cet::demangle_symbol(wanted_wrapper_type.name())
  //    << std::endl;
  std::unique_ptr<art::EDProduct> retval;
  // Try unscheduled production.
  if (fillOnDemand && onDemand()) {
    productProducer_->doWork<OccurrenceTraits<EventPrincipal,
      BranchActionBegin>>(*onDemandPrincipal_, 0);
    return retval;
  }
  BranchKey const bk(productDescription());
  //std::cout
  //    << "calling productResolver_->getProduct(bk, "
  //    << cet::demangle_symbol(wanted_wrapper_type.name())
  //    << ')'
  //    << std::endl;
  retval = productResolver_->getProduct(bk, wanted_wrapper_type);
  //std::cout
  //    << "returning: "
  //    << retval.get()
  //    << std::endl;
  //std::cout
  //    << "-----> End   Group::obtainDesiredProduct(bool, TypeID const&)"
  //    << std::endl;
  return retval;
}

bool
Group::
productUnavailable() const
{
  if (onDemand()) {
    return false;
  }
  if (dropped()) {
    return true;
  }
  if (productstatus::unknown(status())) {
    return false;
  }
  return !productstatus::present(status());
}

cet::exempt_ptr<art::ProductProvenance const>
Group::
productProvenancePtr() const
{
  if (!ppResolver_) {
    return cet::exempt_ptr<ProductProvenance const>();
  }
  return ppResolver_->branchToProductProvenance(branchDescription_->branchID());
}

void
Group::
removeCachedProduct() const
{
  // This check should already have been done by the surrounding art
  // code, therefore this is a precondition and valid for an assertion
  // rather than a runtime check and throw.
  assert(!(branchDescription_ && branchDescription_->produced()));
  product_.reset();
}

void
Group::
setProduct(std::unique_ptr<EDProduct>&& prod) const
{
  assert(!product_.get());
  product_ = std::move(prod);
}

bool
Group::
dropped() const
{
  if ( !branchDescription_ ) return false;
  if (  ProductMetaData::instance().produced( branchDescription_->branchType(),
                                              branchDescription_->branchID() ) ) return false;

  std::size_t const index = ProductMetaData::instance().presentWithFileIdx( branchDescription_->branchType(),
                                                                            branchDescription_->branchID() );
  return index == MasterProductRegistry::DROPPED;
}

void
Group::
swap(Group& other)
{
  using std::swap;
  swap(ppResolver_, other.ppResolver_);
  swap(product_, other.product_);
  swap(branchDescription_, other.branchDescription_);
  swap(pid_, other.pid_);
  swap(productProducer_, other.productProducer_);
  swap(onDemandPrincipal_, other.onDemandPrincipal_);
}

void
Group::
replace(Group& g)
{
  this->swap(g);
}

void
Group::
write(std::ostream& os) const
{
  // This is grossly inadequate. It is also not critical for the
  // first pass.
  os << "Group for product with ID: " << pid_;
}

} // namespace art
