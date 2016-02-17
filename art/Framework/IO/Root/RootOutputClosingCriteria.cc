#include "art/Framework/IO/Root/RootOutputClosingCriteria.h"

bool
art::criteriaMet(ClosingCriteria const& c,
                 unsigned const size,
                 FileIndex::EntryNumber_t const nEvents)
{
  if (size >= c.maxFileSize) return true;
  if (nEvents >= c.maxEventsPerFile) return true;
  return false;
}
