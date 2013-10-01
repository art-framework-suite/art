#include "art/Framework/Services/Registry/ActivityRegistry.h"

// Only local signals are mentioned here: global signals are
// default constructed.
art::ActivityRegistry::ActivityRegistry(size_t numSchedules)
 :
  sPreProcessEvent(numSchedules),
  sPostProcessEvent(numSchedules),
  sPreProcessPath(numSchedules),
  sPostProcessPath(numSchedules),
  sPreModule(numSchedules),
  sPostModule(numSchedules)
{
}
