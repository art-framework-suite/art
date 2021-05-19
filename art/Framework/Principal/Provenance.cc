#include "art/Framework/Principal/Provenance.h"
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/Parentage.h"

#include "fhiclcpp/ParameterSetRegistry.h"

using namespace std;

namespace art {

  Provenance::Provenance() = default;

  Provenance::Provenance(cet::exempt_ptr<Group const> g) noexcept : group_{g} {}

  bool
  Provenance::isValid() const noexcept
  {
    return static_cast<bool>(group_);
  }

  BranchDescription const&
  Provenance::productDescription() const noexcept
  {
    return group_->productDescription();
  }

  bool
  Provenance::produced() const noexcept
  {
    return productDescription().produced();
  }

  string const&
  Provenance::producedClassName() const noexcept
  {
    return productDescription().producedClassName();
  }

  string const&
  Provenance::branchName() const noexcept
  {
    return productDescription().branchName();
  }

  string const&
  Provenance::friendlyClassName() const noexcept
  {
    return productDescription().friendlyClassName();
  }

  string const&
  Provenance::moduleLabel() const noexcept
  {
    return productDescription().moduleLabel();
  }

  string const&
  Provenance::productInstanceName() const noexcept
  {
    return productDescription().productInstanceName();
  }

  string const&
  Provenance::processName() const noexcept
  {
    return productDescription().processName();
  }

  ProductID
  Provenance::productID() const noexcept
  {
    return productDescription().productID();
  }

  set<fhicl::ParameterSetID> const&
  Provenance::psetIDs() const noexcept
  {
    return productDescription().psetIDs();
  }

  fhicl::ParameterSet const&
  Provenance::parameterSet() const
  {
    return fhicl::ParameterSetRegistry::get(
      *productDescription().psetIDs().begin());
  }

  InputTag
  Provenance::inputTag() const
  {
    return productDescription().inputTag();
  }

  ProductStatus
  Provenance::productStatus() const
  {
    return productProvenance().productStatus();
  }

  Parentage const&
  Provenance::parentage() const
  {
    return productProvenance().parentage();
  }

  vector<ProductID> const&
  Provenance::parents() const
  {
    return productProvenance().parentage().parents();
  }

  bool
  Provenance::isPresent() const
  {
    return productstatus::present(productProvenance().productStatus());
  }

  RangeSet const&
  Provenance::rangeOfValidity() const
  {
    return group_->rangeOfValidity();
  }

  bool
  Provenance::equals(Provenance const& other) const noexcept
  {
    return group_ == other.group_;
  }

  ProductProvenance const&
  Provenance::productProvenance() const
  {
    auto prov = group_->productProvenance();
    assert(prov != nullptr);
    return *prov;
  }

  bool
  operator==(Provenance const& a, Provenance const& b) noexcept
  {
    return a.equals(b);
  }

  ostream&
  Provenance::write(ostream& os) const
  {
    // This is grossly inadequate, but it is not critical for the
    // first pass.
    productDescription().write(os);
    productProvenance().write(os);
    return os;
  }

  ostream&
  operator<<(ostream& os, Provenance const& p)
  {
    return p.write(os);
  }

} // namespace art
