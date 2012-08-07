#ifndef art_Framework_Services_UserInteraction_FileTransferStatus_h
#define art_Framework_Services_UserInteraction_FileTransferStatus_h

namespace art {

enum FileTransferStatus {
  CREATED         = 201,  // The only normal return from file transfer protocol
  OK              = 200,
  BADREQUEST      = 400,
  UNAUTHORIZED    = 401,
  PAYMENTREQUIRED = 402,
  FORBIDDEN       = 403,
  NOTFOUND        = 404,
  GONE            = 410,
  TOOLARGE        = 413,
  URITOOLONG      = 404,
  SERVERERROR     = 500,
  UNAVAILABLE     = 503
};

} // end of art namespace

#endif /* art_Framework_Services_UserInteraction_FileTransferStatus_h */
