#ifndef art_Framework_IO_Sources_put_product_in_principal_h
#define art_Framework_IO_Sources_put_product_in_principal_h
// vim: set sw=2 expandtab :

//
// Helper class to put products directly into the {Run, SubRun, Event}
// Principal.
//
// NOTE that this should *not* be used to put products in the event that
// require parentage information to be filled in.
//

#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/get_ProductDescription.h"
#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/ProductStatus.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"

#include <memory>
#include <string>

namespace art {

inline
RangeSet
rangeSetFor(RunPrincipal const& rp)
{
  return RangeSet::forRun(rp.runID());
}

inline
RangeSet
rangeSetFor(SubRunPrincipal const& srp)
{
  return RangeSet::forSubRun(srp.subRunID());
}

template <typename T, typename P>
std::enable_if_t<(P::branch_type == InEvent) || (P::branch_type == InResults)>
put_product_in_principal(std::unique_ptr<T>&& product, P& principal, std::string const& module_label,
                         std::string const& instance_name = {})
{
  if (product.get() == nullptr) {
    TypeID const typeID{typeid(T)};
    throw Exception(errors::NullPointerError)
        << "put_product_in_principal: A null unique_ptr was passed to 'put'.\n"
        << "The pointer is of type "
        << typeID
        << ".\n"
        << "The specified product instance name was '"
        << instance_name
        << "'.\n";
  }

  BranchDescription const& desc = get_ProductDescription<T>(principal, module_label, instance_name);

  std::unique_ptr<EDProduct> wp = std::make_unique<Wrapper<T>>(std::move(product));
  principal.put(desc,
                std::move(std::make_unique<ProductProvenance const>(desc.productID(), productstatus::present())),
                std::move(wp),
                std::move(std::make_unique<RangeSet>()));
}

template <typename T, typename P>
std::enable_if_t<(P::branch_type == InSubRun) || (P::branch_type == InRun)>
put_product_in_principal(std::unique_ptr<T>&& product, P& principal, std::string const& module_label,
                         std::string const& instance_name = {}, RangeSet&& rs = RangeSet::invalid())
{
  if (product.get() == nullptr) {
    TypeID const typeID{typeid(T)};
    throw Exception(errors::NullPointerError)
        << "put_product_in_principal: A null unique_ptr was passed to 'put'.\n"
        << "The pointer is of type "
        << typeID
        << ".\n"
        << "The specified product instance name was '"
        << instance_name
        << "'.\n";
  }

  BranchDescription const& desc = get_ProductDescription<T>(principal, module_label, instance_name);

  // If the provided RangeSet is invalid, assign it a RangeSet
  // corresponding to the full (Sub)Run.
  if (!rs.is_valid()) {
    rs = rangeSetFor(principal);
  }
  std::unique_ptr<EDProduct> wp = std::make_unique<Wrapper<T>>(std::move(product));
  principal.put(desc,
                std::move(std::make_unique<ProductProvenance const>(desc.productID(), productstatus::present())),
                std::move(wp),
                std::move(std::make_unique<RangeSet>(rs)));
}

} // namespace art

#endif /* art_Framework_IO_Sources_put_product_in_principal_h */

// Local Variables:
// mode: c++
// End:
