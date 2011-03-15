#ifndef art_Framework_Core_ProducerBase_h
#define art_Framework_Core_ProducerBase_h

/*----------------------------------------------------------------------

EDProducer: The base class of all "modules" that will insert new
EDProducts into an Event.

----------------------------------------------------------------------*/

#include "art/Framework/Core/ProductRegistryHelper.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include <string>

namespace art {
  class BranchDescription;
  class ModuleDescription;
  class ProductRegistry;
  class ProducerBase : private ProductRegistryHelper
  {
  public:
    ProducerBase ();
    virtual ~ProducerBase();

    /// used by the fwk to register list of products
    typedef boost::function<void(const BranchDescription&)> callback_t;

    callback_t registrationCallback() const;

    void registerProducts(boost::shared_ptr<ProducerBase>,
                        ProductRegistry *,
                        ModuleDescription const&);

    using ProductRegistryHelper::produces;
    using ProductRegistryHelper::typeLabelList;

    bool modifiesEvent() const { return true; }

  protected:
    template<class TProducer, class TMethod>
    void callWhenNewProductsRegistered(TProducer* iProd, TMethod iMethod)
    {
       callWhenNewProductsRegistered_ = boost::bind(iMethod,iProd,_1);
    }

  private:
    callback_t callWhenNewProductsRegistered_;
  };

}  // art

#endif /* art_Framework_Core_ProducerBase_h */

// Local Variables:
// mode: c++
// End:
