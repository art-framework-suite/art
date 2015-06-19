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
#include "art/Persistency/Common/Assns.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/ProductList.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/TypeLabel.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/TypeID.h"
#include "cetlib/exception.h"
#include <memory>
#include <set>
#include <string>

namespace {

inline
void
verifyInstanceName(std::string const& instanceName)
{
  if (instanceName.find('_') != std::string::npos) {
    throw art::Exception(art::errors::Configuration)
        << "Instance name \""
        << instanceName
        << "\" is illegal: underscores are not permitted in instance names."
        << '\n';
  }
}

inline
void
verifyFriendlyClassName(std::string const& fcn)
{
  if (fcn.find('_') != std::string::npos) {
    throw art::Exception(art::errors::LogicError)
        << "Class \""
        << fcn
        << "\" is not suitable for use as a product due to the presence of "
        << "underscores which are not allowed anywhere in the class name "
        << "(including namespace and enclosing classes).\n";
  }
}

} // unnamed namespace


namespace art {

class ModuleDescription;

class ProductRegistryHelper {
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

  using PerBranchTypePresence = MasterProductRegistry::PerBranchTypePresence;

  PerBranchTypePresence
  perBranchTypePresence_( ModuleDescription const& md );

  std::set<TypeLabel> typeLabelList_;

  // Set by an input source for merging into the
  // master product registry by registerProducts().
  std::unique_ptr<ProductList> productList_;

};

template<typename P, BranchType B>
inline
void
ProductRegistryHelper::
produces(std::string const& instanceName)
{
  verifyInstanceName(instanceName);
  TypeID productType(typeid(P));
  verifyFriendlyClassName(productType.friendlyClassName());
  insertOrThrow(TypeLabel(B, productType, instanceName));
}

template<typename P, BranchType B>
TypeLabel const&
ProductRegistryHelper::
reconstitutes(std::string const& emulatedModule,
    std::string const& instanceName)
{
  verifyInstanceName(instanceName);
  TypeID productType(typeid(P));
  verifyFriendlyClassName(productType.friendlyClassName());
  if (emulatedModule.empty()) {
    throw Exception(errors::Configuration)
        << "Input sources must call reconstitutes with a non-empty "
        << "module label.\n";
  }
  return insertOrThrow(TypeLabel(B, productType, instanceName,
                                 emulatedModule));
}

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif // art_Framework_Core_ProductRegistryHelper_h
