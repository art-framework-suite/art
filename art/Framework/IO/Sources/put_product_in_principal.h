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

#include "art/Framework/Principal/RangeSetsSupported.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/ProductStatus.h"
#include "canvas/Persistency/Provenance/canonicalProductName.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"

#include <memory>
#include <string>

namespace art {

  inline RangeSet
  rangeSetFor(RunPrincipal const& rp)
  {
    return RangeSet::forRun(rp.runID());
  }

  inline RangeSet
  rangeSetFor(SubRunPrincipal const& srp)
  {
    return RangeSet::forSubRun(srp.subRunID());
  }

  template <typename T, typename P>
  std::enable_if_t<!detail::range_sets_supported(P::branch_type)>
  put_product_in_principal(std::unique_ptr<T>&& product,
                           P& principal,
                           std::string const& module_label,
                           std::string const& instance_name = {})
  {
    TypeID const typeID{typeid(T)};
    if (product.get() == nullptr) {
      throw Exception(errors::NullPointerError)
        << "put_product_in_principal: A null unique_ptr was passed to 'put'.\n"
        << "The pointer is of type " << typeID << ".\n"
        << "The specified product instance name was '" << instance_name
        << "'.\n";
    }

    auto const& process_name = principal.processConfiguration().processName();
    auto const& product_name = canonicalProductName(
      typeID.friendlyClassName(), module_label, instance_name, process_name);
    ProductID const pid{product_name};
    auto desc = principal.getProductDescription(pid);
    if (!desc) {
      throw art::Exception(art::errors::ProductPutFailure,
                           "put_product_in_principal: error while trying to "
                           "retrieve product description:\n")
        << "No product is registered for\n"
        << "  process name:                '" << process_name << "'\n"
        << "  module label:                '" << module_label << "'\n"
        << "  product friendly class name: '" << typeID.friendlyClassName()
        << "'\n"
        << "  product instance name:       '" << instance_name << "'\n"
        << "  branch type:                 '" << principal.branchType()
        << "'\n";
    }

    std::unique_ptr<EDProduct> wp = std::make_unique<Wrapper<T>>(move(product));
    principal.put(
      *desc,
      std::make_unique<ProductProvenance const>(pid, productstatus::present()),
      move(wp),
      std::make_unique<RangeSet>());
  }

  template <typename T, typename P>
  std::enable_if_t<detail::range_sets_supported(P::branch_type)>
  put_product_in_principal(std::unique_ptr<T>&& product,
                           P& principal,
                           std::string const& module_label,
                           std::string const& instance_name = {},
                           RangeSet&& rs = RangeSet::invalid())
  {
    TypeID const typeID{typeid(T)};
    if (product.get() == nullptr) {
      throw Exception(errors::NullPointerError)
        << "put_product_in_principal: A null unique_ptr was passed to 'put'.\n"
        << "The pointer is of type " << typeID << ".\n"
        << "The specified product instance name was '" << instance_name
        << "'.\n";
    }

    auto const& process_name = principal.processConfiguration().processName();
    auto const& product_name = canonicalProductName(
      typeID.friendlyClassName(), module_label, instance_name, process_name);
    ProductID const pid{product_name};
    auto desc = principal.getProductDescription(pid);
    if (!desc) {
      throw art::Exception(art::errors::ProductPutFailure,
                           "put_product_in_principal: error while trying to "
                           "retrieve product description:\n")
        << "No product is registered for\n"
        << "  process name:                '" << process_name << "'\n"
        << "  module label:                '" << module_label << "'\n"
        << "  product friendly class name: '" << typeID.friendlyClassName()
        << "'\n"
        << "  product instance name:       '" << instance_name << "'\n"
        << "  branch type:                 '" << principal.branchType()
        << "'\n";
    }

    // If the provided RangeSet is invalid, assign it a RangeSet
    // corresponding to the full (Sub)Run.
    if (!rs.is_valid()) {
      rs = rangeSetFor(principal);
    }
    std::unique_ptr<EDProduct> wp = std::make_unique<Wrapper<T>>(move(product));
    principal.put(
      *desc,
      std::make_unique<ProductProvenance const>(pid, productstatus::present()),
      move(wp),
      std::make_unique<RangeSet>(rs));
  }

} // namespace art

#endif /* art_Framework_IO_Sources_put_product_in_principal_h */

// Local Variables:
// mode: c++
// End:
