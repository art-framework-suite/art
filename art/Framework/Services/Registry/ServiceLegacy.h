#ifndef art_Framework_Services_Registry_ServiceLegacy_h
#define art_Framework_Services_Registry_ServiceLegacy_h

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

#endif /* art_Framework_Services_Registry_ServiceLegacy_h */

// Local Variables:
// mode: c++
// End:
