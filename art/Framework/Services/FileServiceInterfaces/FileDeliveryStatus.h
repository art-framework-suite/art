#ifndef art_Framework_Services_FileServiceInterfaces_FileDeliveryStatus_h
#define art_Framework_Services_FileServiceInterfaces_FileDeliveryStatus_h

#include <string>

namespace art {

  namespace detail {
    namespace FDS {
      enum FileDeliveryStatus {
        PENDING = -1,          // Not attempted yet.
        SUCCESS = 0,           // A normal return from delivery protocol.
        TRY_AGAIN_LATER = 202, // A normal return from delivery protocol.
        NO_MORE_FILES = 204,   // A normal return from delivery protocol.
        BAD_REQUEST = 400,
        FORBIDDEN = 403,
        NOT_FOUND = 404,
        CONFLICT = 409,
        SERVER_EXCEPTION = 500,
        BAD_GATEWAY = 502,
        UNAVAILABLE = 503,
        SPECIFIC_ERRORS = 504
      };
    }
  } // namespace detail

  // Enum values must be scoped, eg FileDeliveryStatus::OK.
  using detail::FDS::FileDeliveryStatus;

  // Translate enum to string;
  std::string translateFileDeliveryStatus(FileDeliveryStatus stat);

} // namespace art

#endif /* art_Framework_Services_FileServiceInterfaces_FileDeliveryStatus_h */

// Local Variables:
// mode: c++
// End:
