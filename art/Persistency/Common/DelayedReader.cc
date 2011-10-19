#include "art/Persistency/Common/DelayedReader.h"

art::DelayedReader::~DelayedReader() {}

void
art::DelayedReader::
mergeReaders_(std::shared_ptr<DelayedReader>) {
}

void
art::DelayedReader::
setGroupFinder_(cet::exempt_ptr<EventPrincipal const>) {
}
