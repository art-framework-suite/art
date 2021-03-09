#ifndef art_Framework_Services_Registry_detail_definition_helper_macros_h
#define art_Framework_Services_Registry_detail_definition_helper_macros_h
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/detail/ServiceHelper.h"

#include <memory>

// Define the C-linkage function to  create the helper.
#define DEFINE_ART_SH_CREATE(svc) DEFINE_ART_SH_CREATE_DETAIL(svc, service)

#define DEFINE_ART_SIH_CREATE(svc) DEFINE_ART_SH_CREATE_DETAIL(svc, iface)

#define DEFINE_ART_SH_CREATE_DETAIL(svc, type)                                 \
  EXTERN_C_FUNC_DECLARE_START                                                  \
  std::unique_ptr<art::detail::ServiceHelperBase> create_##type##_helper()     \
  {                                                                            \
    return std::make_unique<art::detail::ServiceHelper<svc>>();                \
  }                                                                            \
  EXTERN_C_FUNC_DECLARE_END

#endif /* art_Framework_Services_Registry_detail_definition_helper_macros_h */

// Local Variables:
// mode: c++
// End:
