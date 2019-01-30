#include "art/Framework/IO/detail/logFileAction.h"
// vim: set sw=2:
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <cstring>
#include <ctime>

void
art::detail::logFileAction(const char* msg, std::string const& file)
{
  time_t t = time(0);
  char ts[] = "dd-Mon-yyyy hh:mm:ss TZN     ";
  struct tm localtm;
  strftime(
    ts, strlen(ts) + 1, "%d-%b-%Y %H:%M:%S %Z", localtime_r(&t, &localtm));
  mf::LogAbsolute("fileAction") << ts << "  " << msg << '\"' << file << '\"';
  mf::FlushMessageLog();
}
