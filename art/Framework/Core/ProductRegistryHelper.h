#ifndef art_Framework_Core_ProductRegistryHelper_h
#define art_Framework_Core_ProductRegistryHelper_h
// vim: set sw=2 expandtab :

// ===================================================================
// This class provides the produces() and reconstitutes() function
// templates used by modules and sources, respectively, to register
// what products they create or read in respectively.
//
// The constructors of an EDProducer or an EDFilter should call
// produces() for each product inserted into a principal.  Instance
// names should be provided only when the module makes more than one
// instance of the same product per event.
//
// The constructors of an InputSource should call reconstitutes() for
// each product if and only if it does not update the
// UpdateOutputCallbacks with a product list.
// ===================================================================

#include "art/Framework/Core/ProducesCollector.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Common/Assns.h"
#include "canvas/Persistency/Common/traits.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/Persistable.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Persistency/Provenance/type_aliases.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib_except/exception.h"

#include <array>
#include <map>
#include <memory>
#include <string>

namespace art {

  class ModuleDescription;

  enum class product_creation_mode { produces, reconstitutes };
  class ProductRegistryHelper {
  public:
    explicit ProductRegistryHelper(product_creation_mode);
    ~ProductRegistryHelper();

    ProductRegistryHelper(ProductRegistryHelper const&) = delete;
    ProductRegistryHelper(ProductRegistryHelper&&) = delete;
    ProductRegistryHelper& operator=(ProductRegistryHelper const&) = delete;
    ProductRegistryHelper& operator=(ProductRegistryHelper&&) = delete;

    // Used by an input source to provide a product list to be merged
    // into the set of products that will be registered.
    void
    productList(std::unique_ptr<ProductList> p)
    {
      productList_ = move(p);
    }

    void registerProducts(ProductDescriptions& productsToRegister,
                          ModuleDescription const& md);

    void fillDescriptions(ModuleDescription const& md);

    // Record the reconstitution of an object of type P, in either the
    // Run, SubRun, or Event, recording that this object was
    // originally created by a module with label modLabel, and with an
    // optional instance name.
    template <typename P, BranchType B>
    TypeLabel const& reconstitutes(std::string const& modLabel,
                                   std::string const& instanceName = {});

    template <BranchType B>
    TypeLabelLookup_t const& expectedProducts() const;

    // Record the production of an object of type P, with optional
    // instance name, in the Event (by default), Run, or SubRun.
    template <typename P, BranchType B = InEvent>
    void produces(std::string const& instanceName = {},
                  Persistable const persistable = Persistable::Yes);

    ProducesCollector&
    producesCollector() noexcept
    {
      return collector_;
    }

  private:
    std::unique_ptr<ProductList const> productList_{nullptr};
    product_creation_mode mode_;
    ProducesCollector collector_;
  };

  template <BranchType B>
  inline TypeLabelLookup_t const&
  ProductRegistryHelper::expectedProducts() const
  {
    return collector_.expectedProducts(B);
  }

  template <typename P, art::BranchType B>
  inline void
  ProductRegistryHelper::produces(std::string const& instanceName,
                                  Persistable const persistable)
  {
    if (mode_ == product_creation_mode::reconstitutes) {
      throw Exception{errors::ProductRegistrationFailure,
                      "An error occurred while registering a product.\n"}
        << "The following product was registered with 'produces' when\n"
           "'reconstitutes' should have been called instead.\n"
        << "  Branch type: " << B << '\n'
        << "  Class name: " << cet::demangle_symbol(typeid(P).name()) << '\n'
        << "  Instance name: '" << instanceName << "'\n";
    }
    collector_.produces<P, B>(instanceName, persistable);
  }

  template <typename P, BranchType B>
  TypeLabel const&
  ProductRegistryHelper::reconstitutes(std::string const& emulatedModule,
                                       std::string const& instanceName)
  {
    if (mode_ == product_creation_mode::produces) {
      throw Exception{errors::ProductRegistrationFailure,
                      "An error occurred while registering a product.\n"}
        << "The following product was registered with 'reconstitutes' when\n"
           "'produces' should have been called instead.\n"
        << "  Branch type: " << B << '\n'
        << "  Class name: " << cet::demangle_symbol(typeid(P).name()) << '\n'
        << "  Emulated module name: '" << emulatedModule << "'\n"
        << "  Instance name: '" << instanceName << "'\n";
    }
    return collector_.reconstitutes<P, B>(emulatedModule, instanceName);
  }

} // namespace art

#endif /* art_Framework_Core_ProductRegistryHelper_h */

// Local Variables:
// mode: c++
// End:
