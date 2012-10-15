#ifndef art_Framework_Services_Registry_detail_ServiceHelper_h
#define art_Framework_Services_Registry_detail_ServiceHelper_h

////////////////////////////////////////////////////////////////////////
// ServiceHelperBase
// ServiceImplHelper
// ServiceInterfaceImplHelper
// ServiceInterfaceHelper
// ServiceLGMHelper
// ServicePSMHelper
// ServiceLGRHelper
// ServicePSRHelper
// ServiceHelper
//
// The ServiceHelper class and associated interface classes to simplify
// the service cache in the face of pre-made and per-schedule services,
// and services implementing interfaces.
//
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Utilities/TypeID.h"

#include "cpp0x/memory"

namespace art {
  namespace detail {
    struct ServiceHelperBase;
    struct ServiceImplHelper;
    struct ServiceInterfaceImplHelper;
    struct ServiceInterfaceHelper;
    struct ServiceLGMHelper;
    struct ServicePSMHelper;
    struct ServiceLGRHelper;
    struct ServicePSRHelper;
    template <typename SERVICE> struct ServiceHelper;
  }
}

// Base class. Note virtual inheritance below.
struct art::detail::ServiceHelperBase {
  virtual ~ServiceHelperBase() = default;
  virtual TypeID get_typeid() const = 0;
  virtual ServiceScope scope() const = 0;
  virtual bool is_interface() const = 0;
  virtual bool is_interface_impl() const = 0;
};

// A "real" service (not an interface).
struct art::detail::ServiceImplHelper :
  public virtual art::detail::ServiceHelperBase {
  bool is_interface() const override { return false; } // Not necessarily final.
};

// A service implementing an interface.
struct art::detail::ServiceInterfaceImplHelper :
  public art::detail::ServiceImplHelper {
  bool is_interface_impl() const final override { return true; }
  virtual TypeID get_interface_typeid() const = 0;

  virtual
  std::unique_ptr<ServiceWrapperBase>
  convert(std::shared_ptr<ServiceWrapperBase> const & swb) const = 0;
};

// An interface.
struct art::detail::ServiceInterfaceHelper :
  public virtual art::detail::ServiceHelperBase {
  bool is_interface() const final override { return true; }
  bool is_interface_impl() const final override { return false; }
};

// A per-schedule service.
struct art::detail::ServicePSMHelper {
  virtual
  std::unique_ptr<ServiceWrapperBase>
  make(fhicl::ParameterSet const & cfg,
       ActivityRegistry & reg,
       size_t nSchedules) const = 0;
};

struct art::detail::ServicePSRHelper {
  virtual
  void *
  retrieve(std::shared_ptr<ServiceWrapperBase> & swb,
           ScheduleID sID) const = 0;
};

// A global or legacy service
struct art::detail::ServiceLGMHelper {
  virtual
  std::unique_ptr<ServiceWrapperBase>
  make(fhicl::ParameterSet const & cfg,
       ActivityRegistry & reg) const = 0;
};

struct art::detail::ServiceLGRHelper {
  virtual
  void *
  retrieve(std::shared_ptr<ServiceWrapperBase> & swb) const = 0;
};

#endif /* art_Framework_Services_Registry_detail_ServiceHelper_h */

// Local Variables:
// mode: c++
// End:
