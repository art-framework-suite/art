#ifndef art_Framework_Core_ProductRegistryHelper_h
#define art_Framework_Core_ProductRegistryHelper_h
// vim: set sw=2:

// -----------------------------------------------------------------
//
// ProductRegistryHelper: This class provides the produces()
// and reconstitutes() function templates used by modules to
// register what products they create or read in respectively.
//
// The constructors of an EDProducer or an EDFilter should call
// produces() for each product inserted into a principal.
// Instance names should be provided only when the module
// makes more than one instance of the same product per event.
//
// The constructors of an InputSource should call reconstitutes()
// for each product if and only if it does not update the
// MasterProductRegistry with a product list.
//
// -----------------------------------------------------------------

#include "art/Framework/Core/detail/verify_names.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/Assns.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ProductList.h"
#include "art/Persistency/Provenance/TypeLabel.h"
#include "art/Persistency/Provenance/detail/type_aliases.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/TypeID.h"
#include "cetlib/exception.h"

#include <memory>
#include <set>
#include <string>

namespace art {
  class ModuleDescription;
  class ProductRegistryHelper;
}

class art::ProductRegistryHelper {
public:

  // Used by an input source to provide a product list
  // to be merged into the master product registry
  // later by registerProducts().
  void productList(ProductList* p) { productList_.reset(p); }

  void registerProducts(MasterProductRegistry& mpr,
                        ModuleDescription const& md);
  // Record the production of an object of type P, with optional
  // instance name, in the Event (by default), Run, or SubRun.
  template<class P, BranchType B = InEvent>
  void produces(std::string const& instanceName = std::string());

  // Record the reconstitution of an object of type P, in either the
  // Run, SubRun, or Event, recording that this object was
  // originally created by a module with label modLabel, and with an
  // optional instance name.
  template<class P, BranchType B>
  TypeLabel const&
  reconstitutes(std::string const& modLabel,
                std::string const& instanceName = std::string());

  template<BranchType B = InEvent>
  ProducedMap const&
  expectedProducts() const
  {
    return expectedProducts_[B];
  }

private:

  TypeLabel const&
  insertOrThrow(TypeLabel const& tl)
  {
    auto result = typeLabelList_.insert(tl);
    if (!result.second) {
      throw Exception(errors::LogicError, "RegistrationFailure")
        << "The module being constructed attempted to "
        << "register conflicting products with:\n"
        << "friendlyClassName: "
        << tl.friendlyClassName()
        << " and instanceName: "
        << tl.productInstanceName
        << ".\n";
    }
    return *result.first;
  }

private:

  std::set<TypeLabel> typeLabelList_;
  PerBranchTypeProduced expectedProducts_;

  // Set by an input source for merging into the
  // master product registry by registerProducts().
  std::unique_ptr<ProductList> productList_;

};

template<typename P, art::BranchType B>
inline
void
art::ProductRegistryHelper::
produces(std::string const& instanceName)
{
  detail::verifyInstanceName(instanceName);
  TypeID productType(typeid(P));
  detail::verifyFriendlyClassName(productType.friendlyClassName());
  insertOrThrow(TypeLabel(B, productType, instanceName));
}

template<typename P, art::BranchType B>
art::TypeLabel const&
art::ProductRegistryHelper::
reconstitutes(std::string const& emulatedModule,
              std::string const& instanceName)
{
  detail::verifyInstanceName(instanceName);
  TypeID productType(typeid(P));
  detail::verifyFriendlyClassName(productType.friendlyClassName());
  if (emulatedModule.empty()) {
    throw Exception(errors::Configuration)
      << "Input sources must call reconstitutes with a non-empty "
      << "module label.\n";
  }
  return insertOrThrow(TypeLabel(B, productType, instanceName,
                                 emulatedModule));
}

#endif /* art_Framework_Core_ProductRegistryHelper_h */

// Local Variables:
// mode: c++
// End:
