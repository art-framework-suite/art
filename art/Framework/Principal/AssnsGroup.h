#ifndef art_Framework_Principal_AssnsGroup_h
#define art_Framework_Principal_AssnsGroup_h

#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Common/Assns.h"
#include "art/Utilities/fwd.h"
#include "cetlib/exempt_ptr.h"

namespace art {

class BranchDescription;
class EDProduct;
class ProductID;

class AssnsGroup : public Group {

  friend
  std::unique_ptr<Group>
  gfactory::make_group(BranchDescription const&,
                       ProductID const&,
                       ProductRangeSetLookup&);

  friend
  std::unique_ptr<Group>
  gfactory::make_group(BranchDescription const&,
                       ProductID const&,
                       ProductRangeSetLookup&,
                       cet::exempt_ptr<Worker>,
                       cet::exempt_ptr<EventPrincipal>);

  friend
  std::unique_ptr<Group>
  gfactory::make_group(std::unique_ptr<EDProduct>&&,
                       BranchDescription const&,
                       ProductID const&,
                       ProductRangeSetLookup&);

public:

  AssnsGroup();

private:

  AssnsGroup(BranchDescription const& bd,
             ProductID const& pid,
             TypeID const& primary_wrapper_type,
             TypeID const& secondary_wrapper_type,
             ProductRangeSetLookup&,
             cet::exempt_ptr<Worker> productProducer = cet::exempt_ptr<Worker>(),
             cet::exempt_ptr<EventPrincipal> onDemandPrincipal =  cet::exempt_ptr<EventPrincipal>());

  AssnsGroup(std::unique_ptr<EDProduct>&& edp,
             BranchDescription const& bd,
             ProductID const& pid,
             TypeID const& primary_type,
             TypeID const& secondary_type,
             ProductRangeSetLookup&);

public:

  bool
  isReady() const override
  {
    return true;
  }

  EDProduct const*
  getIt() const override;

  EDProduct const*
  anyProduct() const override;

  EDProduct const*
  uniqueProduct() const override;

  EDProduct const*
  uniqueProduct(TypeID const& wanted_wrapper_type) const override;

  bool
  resolveProductIfAvailable(bool fillOnDemand, TypeID const&) const override;

private:

  TypeID secondary_wrapper_type_;
  mutable std::unique_ptr<EDProduct> secondaryProduct_;

};

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_Principal_AssnsGroup_h */
