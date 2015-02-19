#include "art/Framework/Services/System/PathSelection.h"

art::PathSelection::
PathSelection(IEventProcessor & ep)
 :
  ep_(ep)
{
}

bool
art::PathSelection::
setTriggerPathEnabled(std::string const & name, bool enable) {
  return ep_.setTriggerPathEnabled(name, enable);
}

bool
art::PathSelection::
setEndPathModuleEnabled(std::string const & label, bool enable) {
  return ep_.setEndPathModuleEnabled(label, enable);
}

// ===============================
PROVIDE_FILE_PATH()
// ===============================
