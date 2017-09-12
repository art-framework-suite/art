#ifndef art_Framework_Core_ProductRegistryHelper_h
#define art_Framework_Core_ProductRegistryHelper_h
// vim: set sw=2 expandtab :

//
// This class provides the produces() and reconstitutes()
// function templates used by modules to register what
// products they create or read in respectively.
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

#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/detail/branchNameComponentChecking.h"
#include "canvas/Persistency/Common/Assns.h"
#include "canvas/Persistency/Common/traits.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "canvas/Persistency/Provenance/ProductTables.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/exception.h"
#include "cetlib/exempt_ptr.h"

#include <array>
#include <memory>
#include <set>
#include <string>

namespace art {

class MasterProductRegistry;
class ModuleDescription;

namespace {

inline
void
verifyFriendlyClassName(std::string const& fcn)
{
  std::string errMsg;
  if (!detail::checkFriendlyName(fcn, errMsg)) {
    throw Exception(errors::Configuration)
      << errMsg
      << "In particular, underscores are not permissible anywhere in the fully-scoped\n"
         "class name, including namespaces.\n";
  }
}

inline
void
verifyModuleLabel(std::string const& ml)
{
  std::string errMsg;
  if (!detail::checkModuleLabel(ml, errMsg)) {
    throw Exception(errors::Configuration)
      << errMsg;
  }
}

inline
void
verifyInstanceName(std::string const& instanceName)
{
  std::string errMsg;
  if (!detail::checkInstanceName(instanceName, errMsg)) {
    throw Exception(errors::Configuration)
      << errMsg;
  }
}

} // unnamed namespace

class ProductRegistryHelper {

public: // MEMBER FUNCTIONS

  virtual
  ~ProductRegistryHelper();

  ProductRegistryHelper();

  ProductRegistryHelper(ProductRegistryHelper const&) = delete;

  ProductRegistryHelper(ProductRegistryHelper&&) = delete;

  ProductRegistryHelper&
  operator=(ProductRegistryHelper const&) = delete;

  ProductRegistryHelper&
  operator=(ProductRegistryHelper&&) = delete;

public: // MEMBER FUNCTIONS

  // Used by an input source to provide a product list to be merged
  // into the master product registry later by registerProducts().
  void
  productList(ProductList* p);

  void registerProducts(MasterProductRegistry& mpr,
                        ProductDescriptions& producedProducts,
                        ModuleDescription const& md);

  // Record the production of an object of type P, with optional
  // instance name, in the Event (by default), Run, or SubRun.
  template <typename P, BranchType B = InEvent>
  void
  produces(std::string const& instanceName = {});

  // Record the reconstitution of an object of type P, in either the
  // Run, SubRun, or Event, recording that this object was
  // originally created by a module with label modLabel, and with an
  // optional instance name.
  template <typename P, BranchType B>
  TypeLabel const&
  reconstitutes(std::string const& modLabel, std::string const& instanceName = {});

  template <BranchType B = InEvent>
  std::set<TypeLabel> const&
  expectedProducts() const;

private: // MEMBER FUNCTIONS

  // FIXME: The following function no longer throws!  Is this intentional?
  TypeLabel const&
  insertOrThrow(BranchType const bt, TypeLabel const& tl);

private: // MEMBER DATA

  std::array<std::set<TypeLabel>, NumBranchTypes>
  typeLabelList_;

  // Set by an input source for merging into the master product
  // registry by registerProducts().  Ownership is released to
  // MasterProductRegistry.
  std::unique_ptr<ProductList> productList_;
};

template <BranchType B = InEvent>
inline
std::set<TypeLabel> const&
ProductRegistryHelper::
expectedProducts() const
{
  return typeLabelList_[B];
}

template<typename P, BranchType B>
inline
void
ProductRegistryHelper::
produces(std::string const& instanceName)
{
  verifyInstanceName(instanceName);
  TypeID const productType{typeid(P)};
  verifyFriendlyClassName(productType.friendlyClassName());
  insertOrThrow(B, TypeLabel{productType, instanceName, SupportsView<P>::value});
}

template<typename P, BranchType B>
TypeLabel const&
ProductRegistryHelper::
reconstitutes(std::string const& emulatedModule, std::string const& instanceName)
{
  verifyModuleLabel(emulatedModule);
  verifyInstanceName(instanceName);
  TypeID const productType{typeid(P)};
  verifyFriendlyClassName(productType.friendlyClassName());
  return insertOrThrow(B, TypeLabel{productType, instanceName, SupportsView<P>::value, emulatedModule});
}

} // namespace art

#endif /* art_Framework_Core_ProductRegistryHelper_h */

// Local Variables:
// mode: c++
// End:
