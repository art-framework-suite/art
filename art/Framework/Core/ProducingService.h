#ifndef art_Framework_Core_ProducingService_h
#define art_Framework_Core_ProducingService_h

#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"

namespace art {

  class ProducingServiceSignals;

  class ProducingService : private ProductRegistryHelper {
  public:
    virtual ~ProducingService() noexcept;

    using ProductRegistryHelper::ProductFlavor;
    using ProductRegistryHelper::produces;
    using ProductRegistryHelper::registerProducts;

    void setModuleDescription(ModuleDescription const& md);

    void registerCallbacks(ProducingServiceSignals&);

  private:
    void doPostReadRun(RunPrincipal& rp);
    void doPostReadSubRun(SubRunPrincipal& srp);
    void doPostReadEvent(EventPrincipal& ep);

    virtual void postReadRun(Run& r);
    virtual void postReadSubRun(SubRun& sr);
    virtual void postReadEvent(Event& e);

    ModuleDescription md_;
  };
}

#define DEFINE_ART_PRODUCING_SERVICE(klass)                                    \
  DECLARE_ART_SERVICE(klass, LEGACY)                                           \
  DEFINE_ART_SERVICE(klass)

#endif /* art_Framework_Core_ProducingService_h */

// Local Variables:
// mode: c++
// End:
