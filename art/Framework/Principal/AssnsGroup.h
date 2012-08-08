#ifndef art_Framework_Principal_AssnsGroup_h
#define art_Framework_Principal_AssnsGroup_h

#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/Assns.h"
#include "art/Utilities/fwd.h"
#include "cetlib/exempt_ptr.h"

namespace art {
  class BranchDescription;
  class EDProduct;
  class ProductID;
}

class art::AssnsGroup : public art::Group {
public:
  AssnsGroup();
private:
  AssnsGroup(BranchDescription const &bd,
             ProductID const &pid,
             art::TypeID const &prinary_wrapper_type,
             art::TypeID const &secondary_wrapper_type,
             cet::exempt_ptr<Worker> productProducer = cet::exempt_ptr<Worker>(),
             cet::exempt_ptr<EventPrincipal> onDemandPrincipal =
             cet::exempt_ptr<EventPrincipal>());
  AssnsGroup(std::unique_ptr<EDProduct> && edp,
             BranchDescription const &bd,
             ProductID const &pid,
             art::TypeID const &prinary_type,
             art::TypeID const &secondary_type);
  friend std::auto_ptr<Group> gfactory::make_group(BranchDescription const&, ProductID const &);
  friend std::auto_ptr<Group> gfactory::make_group(BranchDescription const&, ProductID const &, cet::exempt_ptr<Worker>, cet::exempt_ptr<EventPrincipal>);
  friend std::auto_ptr<Group> gfactory::make_group(std::unique_ptr<EDProduct> &&, BranchDescription const&, ProductID const &);
public:
  virtual ~AssnsGroup();

  virtual EDProduct const *getIt() const;
  virtual EDProduct const *anyProduct() const;
  virtual EDProduct const *uniqueProduct() const;
  virtual EDProduct const *uniqueProduct(TypeID const &wanted_wrapper_type) const;
  virtual bool resolveProductIfAvailable(bool fillOnDemand, TypeID const &) const;

private:
  std::auto_ptr<EDProduct>
  maybeObtainProductFromPartner(TypeID const &wanted_wrapper_type) const;

  art::TypeID secondary_wrapper_type_;
  mutable cet::value_ptr<EDProduct> secondaryProduct_;
};

#endif /* art_Framework_Principal_AssnsGroup_h */

// Local Variables:
// mode: c++
// End:
