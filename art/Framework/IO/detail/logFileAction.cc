#include "art/Framework/IO/detail/logFileAction.h"
// vim: set sw=2:
#include "messagefacility/MessageLogger/MessageLogger.h"
#include <ctime>

void
art::detail::logFileAction(const char* msg, std::string const& file)
{
  time_t t = time(0);
  char ts[] = "dd-Mon-yyyy hh:mm:ss TZN     ";
  strftime(ts, strlen(ts) + 1, "%d-%b-%Y %H:%M:%S %Z", localtime(&t));
  mf::LogAbsolute("fileAction") << ts << "  " << msg << '\"' << file << '\"';
  mf::FlushMessageLog();
}
