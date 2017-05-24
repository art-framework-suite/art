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

#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/detail/branchNameComponentChecking.h"
#include "art/Persistency/Provenance/detail/type_aliases.h"
#include "canvas/Persistency/Common/Assns.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/exception.h"

#include <memory>
#include <set>
#include <string>

namespace art {
  class ProductRegistryHelper;

  class ModuleDescription;
}

namespace {

  inline
  void
  verifyFriendlyClassName(std::string const & fcn)
  {
    std::string errMsg;
    if (!art::detail::checkFriendlyName(fcn, errMsg)) {
      throw art::Exception(art::errors::Configuration)
        << errMsg
        << "In particular, underscores are not permissible anywhere in the fully-scoped\n"
        "class name, including namespaces.\n";
    }
  }

  inline
  void
  verifyModuleLabel(std::string const & ml)
  {
    std::string errMsg;
    if (!art::detail::checkModuleLabel(ml, errMsg)) {
      throw art::Exception(art::errors::Configuration)
        << errMsg;
    }
  }

  inline
  void
  verifyInstanceName(std::string const & instanceName)
  {
    std::string errMsg;
    if (!art::detail::checkInstanceName(instanceName, errMsg)) {
      throw art::Exception(art::errors::Configuration)
        << errMsg;
    }
  }

} // unnamed namespace

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
  template <typename P, BranchType B = InEvent>
  void produces(std::string const& instanceName = std::string());

  // Record the reconstitution of an object of type P, in either the
  // Run, SubRun, or Event, recording that this object was
  // originally created by a module with label modLabel, and with an
  // optional instance name.
  template <typename P, BranchType B>
  TypeLabel const&
  reconstitutes(std::string const& modLabel,
                std::string const& instanceName = std::string());

  template <BranchType B = InEvent>
  std::set<TypeLabel> const&
  expectedProducts() const
  {
    return typeLabelList_[B];
  }

private:

  TypeLabel const&
  insertOrThrow(BranchType const bt, TypeLabel const& tl)
  {
    auto result = typeLabelList_[bt].insert(tl);
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

  std::array<std::set<TypeLabel>, NumBranchTypes> typeLabelList_;

  // Set by an input source for merging into the
  // master product registry by registerProducts().
  std::unique_ptr<ProductList> productList_;
};

template<typename P, art::BranchType B>
inline
void
art::ProductRegistryHelper::produces(std::string const& instanceName)
{
  verifyInstanceName(instanceName);
  TypeID const productType{typeid(P)};
  verifyFriendlyClassName(productType.friendlyClassName());
  insertOrThrow(B, TypeLabel{productType, instanceName});
}

template<typename P, art::BranchType B>
art::TypeLabel const&
art::ProductRegistryHelper::reconstitutes(std::string const& emulatedModule,
                                          std::string const& instanceName)
{
  verifyModuleLabel(emulatedModule);
  verifyInstanceName(instanceName);
  TypeID const productType{typeid(P)};
  verifyFriendlyClassName(productType.friendlyClassName());
  return insertOrThrow(B, TypeLabel{productType, instanceName, emulatedModule});
}

#endif /* art_Framework_Core_ProductRegistryHelper_h */

// Local Variables:
// mode: c++
// End:
