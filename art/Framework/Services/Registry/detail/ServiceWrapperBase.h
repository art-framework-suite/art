#ifndef art_Framework_Services_Registry_detail_ServiceWrapperBase_h
#define art_Framework_Services_Registry_detail_ServiceWrapperBase_h

////////////////////////////////////////////////////////////////////////
// ServiceWrapperBase
//
// Base class through which the framework manages the lifetime of
// ServiceWrapper<T> objects.
//
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>

namespace art {
  namespace detail {
    class ServiceWrapperBase;

    typedef std::shared_ptr<detail::ServiceWrapperBase>  WrapperBase_ptr;
  }
}

#ifndef __CGCCXML__
class art::detail::ServiceWrapperBase {
public:
  ServiceWrapperBase() { }

  // Noncopyable
  ServiceWrapperBase(ServiceWrapperBase const &) = delete;
  ServiceWrapperBase& operator = (ServiceWrapperBase const &) = delete;

  virtual ~ServiceWrapperBase() = default;

  void reconfigure(fhicl::ParameterSet const &);

private:
  virtual void reconfigure_service(fhicl::ParameterSet const &) = 0;
};  // ServiceWrapperBase

inline
void
art::detail::ServiceWrapperBase::reconfigure(fhicl::ParameterSet const & ps)
{
  reconfigure_service(ps);
}

#endif
#endif /* art_Framework_Services_Registry_detail_ServiceWrapperBase_h */

// Local Variables:
// mode: c++
// End:
