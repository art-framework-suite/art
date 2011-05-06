/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

#include "art/Framework/Core/ProducerBase.h"

#include "art/Framework/Core/TypeLabelList.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/ConstProductRegistry.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include <sstream>

namespace art
{

  ProducerBase::ProducerBase() :
    ProductRegistryHelper(),
    callWhenNewProductsRegistered_()
  { }

  ProducerBase::~ProducerBase()
  { }


  typedef std::function<void(const BranchDescription&)> callback_t;

  callback_t
  ProducerBase::registrationCallback() const
  {
    return callWhenNewProductsRegistered_;
  }

  namespace
  {
    class CallbackWrapper {
    public:
      CallbackWrapper(std::shared_ptr<ProducerBase> producer,
                      callback_t callback,
                      ProductRegistry* preg,
                      const ModuleDescription& md) :
        prodbase_(&(*producer)),
        callback_(callback),
        reg_(preg),
        mdesc_(md),
        lastSize_(producer->typeLabelList().size())
      { }

      void operator()(BranchDescription const& md)
      {
        callback_(md);
        addToRegistry();
      }

      void addToRegistry() {
        TypeLabelList& plist = prodbase_->typeLabelList();

        if (lastSize_!=plist.size())
          {
            TypeLabelList::iterator pStart = plist.begin();
            advance(pStart, lastSize_);
            ProductRegistryHelper::addToRegistry(pStart,
                                                 plist.end(),
                                                 mdesc_,
                                                 *reg_);
            lastSize_ = plist.size();
        }
      }

    private:
      ProducerBase*     prodbase_;
      callback_t        callback_;
      ProductRegistry*  reg_;
      ModuleDescription mdesc_;
      size_t            lastSize_;

    };
  }


  void
  ProducerBase::registerProducts(std::shared_ptr<ProducerBase> producer,
                                 ProductRegistry* preg,
                                 ModuleDescription const& md)
  {
    if (typeLabelList().empty() && registrationCallback().empty())
      {
        return;
      }

    //If we have a callback, first tell the callback about all the
    //entries already in the product registry, then add any items this
    //producer wants to add to the registry and only after that do we
    //register the callback. This is done so the callback does not get
    //called for items registered by this producer (avoids circular
    //reference problems)

    bool isListener = false;
    if(!(registrationCallback().empty()))
      {
        isListener=true;
        preg->callForEachBranch(registrationCallback());
      }
    TypeLabelList& plist = typeLabelList();

    ProductRegistryHelper::addToRegistry(plist.begin(),
                                         plist.end(), md,
                                         *(preg), isListener);
    if (!(registrationCallback().empty()))
      {
        ServiceHandle<ConstProductRegistry> regService;
        regService->watchProductAdditions(CallbackWrapper(producer, registrationCallback(), preg, md));
      }
  }

}  // art
