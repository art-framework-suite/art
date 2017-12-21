#ifndef art_Framework_Services_Registry_detail_ServiceWrapperBase_h
#define art_Framework_Services_Registry_detail_ServiceWrapperBase_h
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Provenance/BranchDescription.h"

#include <memory>

namespace art {
  class ModuleDescription;
  class ProducingServiceSignals;
  namespace detail {

    class ServiceWrapperBase {
    public:
      virtual ~ServiceWrapperBase() = default;
      explicit ServiceWrapperBase() = default;

      ServiceWrapperBase(ServiceWrapperBase const&) = delete;
      ServiceWrapperBase(ServiceWrapperBase&&) = delete;
      ServiceWrapperBase& operator=(ServiceWrapperBase const&) = delete;
      ServiceWrapperBase& operator=(ServiceWrapperBase&&) = delete;

      virtual void registerProducts(ProductDescriptions&,
                                    ProducingServiceSignals&,
                                    ModuleDescription const&) = 0;
    };

    using WrapperBase_ptr = std::shared_ptr<ServiceWrapperBase>;

  } // namespace detail
} // namespace art

#endif /* art_Framework_Services_Registry_detail_ServiceWrapperBase_h */

// Local Variables:
// mode: c++
// End:
