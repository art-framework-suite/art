#include "art/Persistency/Common/DelayedReader.h"

using namespace std;

namespace art {

DelayedReader::
~DelayedReader()
{
}

void
DelayedReader::
setGroupFinder_(cet::exempt_ptr<EDProductGetterFinder const>)
{
}

void
DelayedReader::
mergeReaders_(shared_ptr<DelayedReader>)
{
}

int
DelayedReader::
openNextSecondaryFile_(int /*idx*/)
{
  return -2;
}

} // namespace art

