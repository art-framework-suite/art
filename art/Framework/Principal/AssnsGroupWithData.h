#ifndef art_Framework_Principal_AssnsGroupWithData_h
#define art_Framework_Principal_AssnsGroupWithData_h

#include "art/Framework/Principal/AssnsGroup.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/fwd.h"
#include "canvas/Persistency/Common/Assns.h"
#include "cetlib/exempt_ptr.h"

namespace art {
  class AssnsGroupWithData;
}

class art::AssnsGroupWithData : public AssnsGroup {

  // The operative part of the GroupFactory system.
  template <typename... ARGS>
  friend std::unique_ptr<Group> gfactory::detail::make_group(
    BranchDescription const&,
    ARGS&&... args);

public:
  AssnsGroupWithData() = default;

protected:
  AssnsGroupWithData(
    BranchDescription const& pd,
    ProductID const& pid,
    RangeSet&& rs,
    TypeID const& primary_wrapper_type,
    TypeID const& partner_wrapper_type,
    TypeID const& base_wrapper_type,
    TypeID const& partner_base_wrapper_type,
    std::unique_ptr<EDProduct>&& edp = nullptr,
    cet::exempt_ptr<Worker> productProducer = cet::exempt_ptr<Worker>())
    : AssnsGroup(pd,
                 pid,
                 std::move(rs),
                 primary_wrapper_type,
                 partner_wrapper_type,
                 std::move(edp),
                 productProducer)
    , baseWrapperType_{base_wrapper_type}
    , partnerBaseWrapperType_{partner_base_wrapper_type}
    , baseProduct_{}
    , partnerBaseProduct_{}
  {}

  AssnsGroupWithData(BranchDescription const& pd,
                     ProductID const& pid,
                     RangeSet&& rs,
                     std::unique_ptr<EDProduct>&& edp,
                     TypeID const& primary_wrapper_type,
                     TypeID const& partner_wrapper_type,
                     TypeID const& base_wrapper_type,
                     TypeID const& partner_base_wrapper_type)
    : AssnsGroupWithData(pd,
                         pid,
                         std::move(rs),
                         primary_wrapper_type,
                         partner_wrapper_type,
                         base_wrapper_type,
                         partner_base_wrapper_type,
                         std::move(edp))
  {}

  AssnsGroupWithData(BranchDescription const& pd,
                     ProductID const& pid,
                     RangeSet&& rs,
                     cet::exempt_ptr<Worker> productProducer,
                     TypeID const& primary_wrapper_type,
                     TypeID const& partner_wrapper_type,
                     TypeID const& base_wrapper_type,
                     TypeID const& partner_base_wrapper_type)
    : AssnsGroupWithData(pd,
                         pid,
                         std::move(rs),
                         primary_wrapper_type,
                         partner_wrapper_type,
                         base_wrapper_type,
                         partner_base_wrapper_type,
                         nullptr,
                         productProducer)
  {}

  EDProduct const* anyProduct() const override;

  using AssnsGroup::uniqueProduct;

  EDProduct const* uniqueProduct(
    TypeID const& wanted_wrapper_type) const override;

  bool resolveProductIfAvailable(TypeID const&) const override;

  void removeCachedProduct() const override;

private:
  TypeID baseWrapperType_;
  TypeID partnerBaseWrapperType_;
  mutable std::unique_ptr<EDProduct> baseProduct_;
  mutable std::unique_ptr<EDProduct> partnerBaseProduct_;
};

#endif /* art_Framework_Principal_AssnsGroupWithData_h */

// Local Variables:
// mode: c++
// End:
