#ifndef art_Framework_Services_Interfaces_FileDeliveryStatus_h
#define art_Framework_Services_Interfaces_FileDeliveryStatus_h

namespace art {

enum FileDeliveryStatus {
  SUCCESS           = 0,             // A normal return from delivery protocol
  TRY_AGAIN_LATER   = 202,
  NO_MORE_FILES     = 204,           // A normal return from delivery protocol
  BAD_REQUEST       = 400,
  FORBIDDEN         = 403,
  NOT_FOUND         = 404,
  CONFLICT          = 409,
  SERVER_EXCEPTION  = 500,
  BAD_GATEWAY       = 502,
  UNAVAILABLE       = 503,
  SPECIFIC_ERRORS   = 504  
};

} // end of art namespace

#endif /* art_Framework_Services_Interfaces_FileDeliveryStatus_h */

// Local Variables:
// mode: c++
// End:
