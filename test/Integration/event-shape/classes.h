#include "test/Integration/event-shape/ESPtrSimple.h"
#include "art/Persistency/Common/Wrapper.h"
#include "art/Persistency/Common/Ptr.h"

template class art::Ptr<arttest::Simple>;
template class art::Wrapper<arttest::ESPtrSimple>;
