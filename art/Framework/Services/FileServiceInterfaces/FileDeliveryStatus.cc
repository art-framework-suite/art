#include "art/Framework/Services/FileServiceInterfaces/FileDeliveryStatus.h"

#include <iomanip>
#include <sstream>
#include <string>

std::string
art::translateFileDeliveryStatus(FileDeliveryStatus stat)
{
  switch( stat ) {
  default:
  {
    std::ostringstream os;
    os << "UNKNOWN"
       << std::setfill('0')
       << std::setw(3)
       << static_cast<int>(stat);
    return os.str();
  }
  case FileDeliveryStatus::PENDING:
    return "PENDING";
  case FileDeliveryStatus::SUCCESS:
    return "SUCCESS";
  case FileDeliveryStatus::TRY_AGAIN_LATER:
    return "TRY_AGAIN_LATER";
  case FileDeliveryStatus::NO_MORE_FILES:
    return "NO_MORE_FILES";
  case FileDeliveryStatus::BAD_REQUEST:
    return "BAD_REQUEST";
  case FileDeliveryStatus::FORBIDDEN:
    return "FORBIDDEN";
  case FileDeliveryStatus::NOT_FOUND:
    return "NOT_FOUND";
  case FileDeliveryStatus::CONFLICT:
    return "CONFLICT";
  case FileDeliveryStatus::SERVER_EXCEPTION:
    return "SERVER_EXCEPTION";
  case FileDeliveryStatus::BAD_GATEWAY:
    return "BAD_GATEWAY";
  case FileDeliveryStatus::UNAVAILABLE:
    return "UNAVAILABLE";
  case FileDeliveryStatus::SPECIFIC_ERRORS:
    return "SPECIFIC_ERRORS";
  }
}
