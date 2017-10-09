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
//   LG = Legacy/Global  M = make  R = retrieve
//   PS = Per-schedule   M = make  R = retrieve
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapperBase.h"
#include "art/Utilities/ScheduleID.h"
#include "canvas/Utilities/TypeID.h"

#include <memory>

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

  class ActivityRegistry;
}

// Base class. Note virtual inheritance below.
struct art::detail::ServiceHelperBase {
  virtual ~ServiceHelperBase() noexcept = default;
  virtual TypeID get_typeid() const = 0;
  virtual ServiceScope scope() const = 0;
  virtual bool is_interface() const = 0;
  virtual bool is_interface_impl() const = 0;
};

// For a "real" service (not an interface).
struct art::detail::ServiceImplHelper :
  public virtual art::detail::ServiceHelperBase {
  bool is_interface() const override { return false; } // Not necessarily final.
};

// For a service implementing an interface.
struct art::detail::ServiceInterfaceImplHelper :
  public art::detail::ServiceImplHelper {
  bool is_interface_impl() const final override { return true; }
  virtual TypeID get_interface_typeid() const = 0;

  virtual
  std::unique_ptr<ServiceWrapperBase>
  convert(std::shared_ptr<ServiceWrapperBase> const & swb) const = 0;
};

// For the service interface itself.
struct art::detail::ServiceInterfaceHelper :
  public virtual art::detail::ServiceHelperBase {
  bool is_interface() const final override { return true; }
  bool is_interface_impl() const final override { return false; }
};

// For a per-schedule service.
struct art::detail::ServicePSMHelper { // PerScheduleMaker
  virtual ~ServicePSMHelper() noexcept = default;
  virtual
  std::unique_ptr<ServiceWrapperBase>
  make(fhicl::ParameterSet const & cfg,
       ActivityRegistry & reg,
       size_t nSchedules) const = 0;
};

struct art::detail::ServicePSRHelper { // PerScheduleRetriever
  virtual ~ServicePSRHelper() noexcept = default;
  virtual
  void *
  retrieve(std::shared_ptr<ServiceWrapperBase> & swb,
           ScheduleID sID) const = 0;
};

// For a global or legacy service
struct art::detail::ServiceLGMHelper { // LegacyOrGlobalMaker
  virtual ~ServiceLGMHelper() noexcept = default;
  virtual
  std::unique_ptr<ServiceWrapperBase>
  make(fhicl::ParameterSet const & cfg,
       ActivityRegistry & reg) const = 0;
};

struct art::detail::ServiceLGRHelper { // LegacyOrGlobalRetriever
  virtual ~ServiceLGRHelper() noexcept = default;
  virtual
  void *
  retrieve(std::shared_ptr<ServiceWrapperBase> & swb) const = 0;
};

#endif /* art_Framework_Services_Registry_detail_ServiceHelper_h */

// Local Variables:
// mode: c++
// End:
