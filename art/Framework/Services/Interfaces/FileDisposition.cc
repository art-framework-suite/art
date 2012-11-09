#include "art/Framework/Services/Interfaces/FileDisposition.h"

#include <iomanip>
#include <sstream>
#include <string>

std::string
art::translateFileDisposition(FileDisposition fd)
{
  switch( fd ) {
  default:
  {
    std::ostringstream os;
    os << "UNKNOWN"
       << std::setfill('0')
       << std::setw(3)
       << static_cast<int>(fd);
    return os.str();
  }
  case FileDisposition::PENDING:
    return "PENDING";
  case FileDisposition::TRANSFERRED:
    return "TRANSFERED";
  case FileDisposition::CONSUMED:
    return "CONSUMED";
  case FileDisposition::SKIPPED:
    return "SKIPPED";
  case FileDisposition::INCOMPLETE:
    return "INCOMPLETE";
  }
}
