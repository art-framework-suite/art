#ifndef art_Framework_Core_ProducingService_h
#define art_Framework_Core_ProducingService_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "art/Persistency/Provenance/ModuleDescription.h"

namespace art {

  class ProducingServiceSignals;

  class ProducingService : private ProductRegistryHelper {
  public: // Types
    static constexpr bool service_handle_allowed{false};

  public: // Special Member Functions
    virtual ~ProducingService() noexcept;

  public: // API
    using ProductRegistryHelper::produces;
    using ProductRegistryHelper::registerProducts;
    void setModuleDescription(ModuleDescription const&);
    void registerCallbacks(ProducingServiceSignals&);

  private: // The signal handlers to register.
    void doPostReadRun(RunPrincipal&);
    void doPostReadSubRun(SubRunPrincipal&);
    void doPostReadEvent(EventPrincipal&);

  private: // API derived classes can implement.
    virtual void postReadRun(Run&);
    virtual void postReadSubRun(SubRun&);
    virtual void postReadEvent(Event&);

  private: // Member Data
    // The fake module description created by the service mgr
    // which contains the service_type as the module label.
    // We must copy it because it has no permanent existence.
    ModuleDescription md_;
  };

} // namespace art

#define DEFINE_ART_PRODUCING_SERVICE(klass)                                    \
  DECLARE_ART_SERVICE(klass, LEGACY)                                           \
  DEFINE_ART_SERVICE(klass)

#endif /* art_Framework_Core_ProducingService_h */

// Local Variables:
// mode: c++
// End:
