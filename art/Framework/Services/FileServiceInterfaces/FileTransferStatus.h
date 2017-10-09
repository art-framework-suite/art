#ifndef art_Framework_Services_FileServiceInterfaces_FileTransferStatus_h
#define art_Framework_Services_FileServiceInterfaces_FileTransferStatus_h

#include <string>

namespace art {

  namespace detail {
    namespace FTS {
      enum FileTransferStatus {
        PENDING = -1, // Not attempted yet.
        SUCCESS = 0,  // A normal return from delivery protocol
        BAD_REQUEST = 400,
        UNAUTHORIZED = 401,
        PAYMENT_REQUIRED = 402,
        FORBIDDEN = 403,
        NOT_FOUND = 404,
        GONE = 410,
        TOO_LARGE = 413,
        URI_TOO_LONG = 414,
        SERVER_ERROR = 500,
        UNAVAILABLE = 503
      };
    }
  }

  // Enum values must be scoped, eg FileDeliveryStatus::OK.
  using detail::FTS::FileTransferStatus;

  // Translate enum to string;
  std::string translateFileTransferStatus(FileTransferStatus stat);

} // end of art namespace

#endif /* art_Framework_Services_FileServiceInterfaces_FileTransferStatus_h */

// Local Variables:
// mode: c++
// End:
