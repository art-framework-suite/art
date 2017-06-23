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

  // The operative part of the GroupFactory system.
  template <typename ... ARGS>
  friend
  std::unique_ptr<Group>
  gfactory::detail::
  make_group(BranchDescription const &, ARGS && ... args);

public:

  AssnsGroup() = default;

protected:

  AssnsGroup(BranchDescription const& pd,
             ProductID const& pid,
             RangeSet&& rs,
             TypeID const& primary_wrapper_type,
             TypeID const& partner_wrapper_type,
             std::unique_ptr<EDProduct>&& edp = nullptr,
             cet::exempt_ptr<Worker> productProducer = cet::exempt_ptr<Worker>())
    : Group{pd, pid, std::move(rs), primary_wrapper_type, std::move(edp), productProducer}
    , partnerWrapperType_{partner_wrapper_type}
    , partnerProduct_{}
    {}

  AssnsGroup(BranchDescription const& pd,
             ProductID const& pid,
             RangeSet&& rs,
             std::unique_ptr<EDProduct>&& edp,
             TypeID const& primary_wrapper_type,
             TypeID const& partner_wrapper_type)
    : AssnsGroup{pd, pid, std::move(rs), primary_wrapper_type, partner_wrapper_type, std::move(edp)}
    {}

  AssnsGroup(BranchDescription const& pd,
             ProductID const& pid,
             RangeSet&& rs,
             cet::exempt_ptr<Worker> productProducer,
             TypeID const& primary_wrapper_type,
             TypeID const& partner_wrapper_type)
    : AssnsGroup{pd, pid, std::move(rs), primary_wrapper_type, partner_wrapper_type, nullptr, productProducer}
    {}

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
  resolveProductIfAvailable(TypeID const&) const override;

  void removeCachedProduct() const override;

protected:

  TypeID const & partnerWrapperType() const
    {
      return partnerWrapperType_;
    }

  bool makePartner(TypeID const& wanted_wrapper_type,
                   std::unique_ptr<EDProduct> & partner) const
    {
      bool result;
      auto edp = Group::uniqueProduct()->makePartner(wanted_wrapper_type.typeInfo());
      if ((result = edp.get())) {
        partner = std::move(edp);
      }
      return result;
    }

private:

  TypeID partnerWrapperType_;
  mutable std::unique_ptr<EDProduct> partnerProduct_;

};

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_Principal_AssnsGroup_h */
