#include "art/Framework/Services/FileServiceInterfaces/FileTransferStatus.h"

#include <iomanip>
#include <sstream>
#include <string>

std::string
art::translateFileTransferStatus(FileTransferStatus stat)
{
  switch (stat) {
  default: {
    std::ostringstream os;
    os << "UNKNOWN" << std::setfill('0') << std::setw(3)
       << static_cast<int>(stat);
    return os.str();
  }
  case FileTransferStatus::PENDING:
    return "PENDING";
  case FileTransferStatus::SUCCESS:
    return "SUCCESS";
  case FileTransferStatus::BAD_REQUEST:
    return "BAD_REQUEST";
  case FileTransferStatus::UNAUTHORIZED:
    return "UNAUTHORIZED";
  case FileTransferStatus::PAYMENT_REQUIRED:
    return "PAYMENT_REQUIRED";
  case FileTransferStatus::FORBIDDEN:
    return "FORBIDDEN";
  case FileTransferStatus::NOT_FOUND:
    return "NOT_FOUND";
  case FileTransferStatus::GONE:
    return "GONE";
  case FileTransferStatus::TOO_LARGE:
    return "TOO_LARGE";
  case FileTransferStatus::URI_TOO_LONG:
    return "URI_TOO_LONG";
  case FileTransferStatus::SERVER_ERROR:
    return "SERVER_ERROR";
  case FileTransferStatus::UNAVAILABLE:
    return "UNAVAILABLE";
  }
}
