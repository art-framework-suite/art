#include "art/Framework/Principal/Provenance.h"
// vim: set sw=2 expandtab :

#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"

using namespace std;

namespace art {

Provenance::
~Provenance()
{
}

Provenance::
Provenance()
  : group_{nullptr}
{
}

Provenance::
Provenance(cet::exempt_ptr<Group const> g)
  : group_{g}
{
}

bool
Provenance::
isValid() const
{
  return static_cast<bool>(group_);
}

BranchDescription const&
Provenance::
productDescription() const
{
  return group_->productDescription();
}

bool
Provenance::
produced() const
{
  return group_->productDescription().produced();
}

string const&
Provenance::
producedClassName() const
{
  return group_->productDescription().producedClassName();
}

string const&
Provenance::
branchName() const
{
  return group_->productDescription().branchName();
}

string const&
Provenance::
friendlyClassName() const
{
  return group_->productDescription().friendlyClassName();
}

string const&
Provenance::
moduleLabel() const
{
  return group_->productDescription().moduleLabel();
}

string const&
Provenance::
productInstanceName() const
{
  return group_->productDescription().productInstanceName();
}

string const&
Provenance::
processName() const
{
  return group_->productDescription().processName();
}

ProductID
Provenance::
productID() const
{
  return group_->productDescription().productID();
}

set<fhicl::ParameterSetID> const&
Provenance::
psetIDs() const
{
  return group_->productDescription().psetIDs();
}

fhicl::ParameterSet const&
Provenance::
parameterSet() const
{
  return fhicl::ParameterSetRegistry::get(*group_->productDescription().psetIDs().begin());
}

InputTag
Provenance::
inputTag() const
{
  return InputTag{group_->productDescription().moduleLabel(), group_->productDescription().productInstanceName(),
                  group_->productDescription().processName()};
}

ProductStatus const&
Provenance::
productStatus() const
{
  return group_->productProvenance()->productStatus();
}

Parentage const&
Provenance::
parentage() const
{
  return group_->productProvenance()->parentage();
}

vector<ProductID> const&
Provenance::
parents() const
{
  return group_->productProvenance()->parentage().parents();
}

bool
Provenance::
isPresent() const
{
  return productstatus::present(group_->productProvenance()->productStatus());
}

RangeSet const&
Provenance::
rangeOfValidity() const
{
  return group_->rangeOfValidity();
}

bool
Provenance::
equals(Provenance const& other) const
{
  return group_ == other.group_;
}

bool
operator==(Provenance const& a, Provenance const& b)
{
  return a.equals(b);
}

ostream&
Provenance::
write(ostream& os) const
{
  // This is grossly inadequate, but it is not critical for the
  // first pass.
  group_->productDescription().write(os);
  group_->productProvenance()->write(os);
  return os;
}

ostream&
operator<<(ostream& os, Provenance const& p)
{
  return p.write(os);
}

} // namespace art
