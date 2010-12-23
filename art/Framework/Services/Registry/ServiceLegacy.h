#ifndef ServiceRegistry_ServiceLegacy_h
#define ServiceRegistry_ServiceLegacy_h

// ======================================================================
//
// ServiceLegacy - Enumeration of how Services inherit from other Service
//                 sets
//
// ======================================================================

namespace art {
  enum ServiceLegacy { kOverlapIsError
                     , kTokenOverrides
                     , kConfigurationOverrides
                     };
}

// ======================================================================

#endif  // ServiceRegistry_ServiceLegacy_h
