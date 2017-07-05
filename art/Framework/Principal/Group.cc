#include "art/Framework/Principal/Group.h"
// vim: set sw=2:

#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/PrincipalPackages.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Persistency/Provenance/ProductStatus.h"
#include "canvas/Persistency/Provenance/TypeTools.h"
#include "cetlib_except/demangle.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <string>

art::ProductStatus
art::Group::status() const
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
art::Group::resolveProduct(TypeID const& wanted_wrapper_type) const
{
  if (!productUnavailable()) {
    return resolveProductIfAvailable(wanted_wrapper_type);
  }
  art::Exception e {errors::ProductNotFound, "InaccessibleProduct"};
  e << "resolveProduct: product is not accessible\n"
    << productDescription()
    << '\n';
  if (productProvenancePtr()) {
    e << *productProvenancePtr() << '\n';
  }
  throw e;
}

bool
art::Group::resolveProductIfAvailable(TypeID const& wanted_wrapper_type) const
{
  if (wanted_wrapper_type != wrapperType_) {
    throwResolveLogicError(wanted_wrapper_type);
  }
  bool result = product_.get();
  if (!(result || productUnavailable())) {
    std::unique_ptr<EDProduct> edp {obtainDesiredProduct(wanted_wrapper_type)};
    if ((result = edp.get())) {
      setProduct(std::move(edp));
    }
  }
  return result;
}

std::unique_ptr<art::EDProduct>
art::Group::obtainDesiredProduct(TypeID const& wanted_wrapper_type) const
{
  BranchKey const bk {productDescription()};
  return productResolver_->getProduct(bk,
                                      wanted_wrapper_type,
                                      rangeOfValidity_);
}

bool
art::Group::productUnavailable() const
{
  if (dropped()) {
    return true;
  }
  if (productstatus::unknown(status())) {
    return false;
  }
  return !productstatus::present(status());
}

cet::exempt_ptr<art::ProductProvenance const>
art::Group::productProvenancePtr() const
{
  if (!ppResolver_) {
    return cet::exempt_ptr<ProductProvenance const>{};
  }
  return ppResolver_->branchToProductProvenance(branchDescription_->productID());
}

void
art::Group::removeCachedProduct() const
{
  // This check should already have been done by the surrounding art
  // code, therefore this is a precondition and valid for an assertion
  // rather than a runtime check and throw.
  assert(!(branchDescription_ && branchDescription_->produced()));
  product_.reset();
}

void
art::Group::setProduct(std::unique_ptr<EDProduct>&& prod) const
{
  assert(!product_.get());
  product_ = std::move(prod);
}

void
art::Group::throwResolveLogicError(TypeID const & wanted_wrapper_type) const
{
  throw Exception(errors::LogicError, "INTERNAL ERROR: ")
    << cet::demangle_symbol(typeid(*this).name())
    << " cannot resolve wanted product of type "
    << wanted_wrapper_type.className()
    << ".\n";
}

bool
art::Group::dropped() const
{
  if (!branchDescription_ ) return false;
  auto const pid = branchDescription_->productID();
  if (ProductMetaData::instance().produced(branchDescription_->branchType(), pid)) return false;

  std::size_t const index = ProductMetaData::instance().presentWithFileIdx(branchDescription_->branchType(), pid);
  return index == MasterProductRegistry::DROPPED;
}

void
art::Group::write(std::ostream& os) const
{
  // This is grossly inadequate. It is also not critical for the
  // first pass.
  os << "Group for product with ID: " << pid_;
}
