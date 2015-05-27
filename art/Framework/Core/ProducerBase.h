#ifndef art_Framework_Core_ProducerBase_h
#define art_Framework_Core_ProducerBase_h

/*----------------------------------------------------------------------

EDProducer: The base class of all "modules" that will insert new
EDProducts into an Event.

----------------------------------------------------------------------*/

#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/Core/get_BranchDescription.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/ProductID.h"

#include <functional>
#include <memory>
#include <string>

namespace art {
  class BranchDescription;
  class ModuleDescription;
  class MasterProductRegistry;
  class ProducerBase : private ProductRegistryHelper
  {
  public:
    virtual ~ProducerBase();

    using ProductRegistryHelper::registerProducts;
    using ProductRegistryHelper::produces;

    bool modifiesEvent() const { return true; }

    template <typename PROD, BranchType B, typename TRANS>
    ProductID getProductID(TRANS const &translator,
                           ModuleDescription const &moduleDescription,
                           std::string const& instanceName) const;
  };

  template <typename PROD, BranchType B, typename TRANS>
  ProductID
  ProducerBase::getProductID(TRANS const &translator,
                             ModuleDescription const &md,
                             std::string const &instanceName) const {
    return
      translator.branchIDToProductID
      (get_BranchDescription<PROD>(B,
                                   md.moduleLabel(),
                                   instanceName).branchID());

  }

}  // art

#endif /* art_Framework_Core_ProducerBase_h */

// Local Variables:
// mode: c++
// End:
