#ifndef art_Framework_Services_System_PathSelection_h
#define art_Framework_Services_System_PathSelection_h

#include "art/Framework/Core/IEventProcessor.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"

#include <string>

namespace art {
  class PathSelection;
}

class art::PathSelection {
public:
  explicit PathSelection(IEventProcessor & ep);

  bool setTriggerPathEnabled(std::string const & name, bool enable);
  bool setEndPathModuleEnabled(std::string const & label, bool enable);

private:
  IEventProcessor & ep_;
};

DECLARE_ART_SYSTEM_SERVICE(art::PathSelection, LEGACY)
#endif /* art_Framework_Services_System_PathSelection_h */

// Local Variables:
// mode: c++
// End:
