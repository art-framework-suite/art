#ifndef art_Framework_Services_Registry_detail_ServiceWrapperBase_h
#define art_Framework_Services_Registry_detail_ServiceWrapperBase_h

////////////////////////////////////////////////////////////////////////
// ServiceWrapperBase
//
// Base class through which the framework manages the lifetime of
// ServiceWrapper<T> objects.
//
////////////////////////////////////////////////////////////////////////

#include "canvas/Persistency/Provenance/BranchDescription.h"

#include <memory>

namespace art {
  namespace detail {
    class ServiceWrapperBase;
    using WrapperBase_ptr = std::shared_ptr<detail::ServiceWrapperBase>;
  }
  class MasterProductRegistry;
  class ModuleDescription;
  class ProducingServiceSignals;
}

class art::detail::ServiceWrapperBase {
public:
  explicit ServiceWrapperBase() = default;

  // Noncopyable
  ServiceWrapperBase(ServiceWrapperBase const&) = delete;
  ServiceWrapperBase& operator=(ServiceWrapperBase const&) = delete;

  virtual void registerProducts(MasterProductRegistry&,
                                ProductDescriptions&,
                                ProducingServiceSignals&,
                                ModuleDescription const&) = 0;

  virtual ~ServiceWrapperBase() = default;

}; // ServiceWrapperBase

#endif /* art_Framework_Services_Registry_detail_ServiceWrapperBase_h */

// Local Variables:
// mode: c++
// End:
