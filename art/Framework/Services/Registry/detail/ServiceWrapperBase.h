#ifndef art_Framework_Services_Registry_detail_ServiceWrapperBase_h
#define art_Framework_Services_Registry_detail_ServiceWrapperBase_h
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>

namespace art {
namespace detail {

class ServiceWrapperBase {

public:

  virtual
  ~ServiceWrapperBase() = default;

  explicit
  ServiceWrapperBase() = default;

  ServiceWrapperBase(ServiceWrapperBase const&) = delete;

  ServiceWrapperBase(ServiceWrapperBase&&) = delete;

  ServiceWrapperBase&
  operator=(ServiceWrapperBase const&) = delete;

  ServiceWrapperBase&
  operator=(ServiceWrapperBase&&) = delete;

};

using WrapperBase_ptr = std::shared_ptr<ServiceWrapperBase>;

} // namespace detail
} // namespace art

#endif /* art_Framework_Services_Registry_detail_ServiceWrapperBase_h */

// Local Variables:
// mode: c++
// End:
