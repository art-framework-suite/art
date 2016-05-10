#include "art/Framework/IO/Root/RootOutputClosingCriteria.h"

bool
art::criteriaMet(ClosingCriteria const& c,
                 unsigned const size,
                 FileIndex::EntryNumber_t const nEvents,
                 std::chrono::seconds const seconds)
{
  if (size >= c.maxFileSize) return true;
  if (nEvents >= c.maxEventsPerFile) return true;
  if (seconds >= c.maxFileAge) return true;
  return false;
}
