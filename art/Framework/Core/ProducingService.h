#ifndef art_Framework_Core_ProducingService_h
#define art_Framework_Core_ProducingService_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "art/Persistency/Provenance/ModuleContext.h"

namespace art {

  class ModuleDescription;
  class ProducingServiceSignals;

  class ProducingService : private ProductRegistryHelper {
  public:
    static constexpr bool service_handle_allowed{false};

    virtual ~ProducingService() noexcept;

    using ProductRegistryHelper::produces;
    using ProductRegistryHelper::registerProducts;
    void setModuleDescription(ModuleDescription const&);
    void registerCallbacks(ProducingServiceSignals&);

    // The signal handlers to register.
    void doPostReadRun(RunPrincipal&);
    void doPostReadSubRun(SubRunPrincipal&);
    void doPostReadEvent(EventPrincipal&);

  private:
    // Derived classes can implement these.
    virtual void postReadRun(Run&);
    virtual void postReadSubRun(SubRun&);
    virtual void postReadEvent(Event&);

    // The fake module description/context created by the service mgr
    // which contains the service_type as the module label.  We must
    // copy it because it has no permanent existence.
    ModuleContext mc_{ModuleContext::invalid()};
  };

} // namespace art

#define DEFINE_ART_PRODUCING_SERVICE(klass)                                    \
  DECLARE_ART_SERVICE(klass, GLOBAL)                                           \
  DEFINE_ART_SERVICE(klass)

#endif /* art_Framework_Core_ProducingService_h */

// Local Variables:
// mode: c++
// End:
