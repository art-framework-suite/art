#ifndef art_Framework_Services_System_CurrentModule_h
#define art_Framework_Services_System_CurrentModule_h

// ======================================================================
//
// CurrentModule: A Service to track and make available information re
//                the currently-running module
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include <string>

namespace art {
  class ActivityRegistry; // declaration only
  class CurrentModule;    // defined below
}

// ----------------------------------------------------------------------

class art::CurrentModule {

  CurrentModule(CurrentModule const&) = delete;
  CurrentModule operator=(CurrentModule const&) = delete;

public:
  CurrentModule(art::ActivityRegistry& r);

  std::string const&
  label() const
  {
    return desc_.moduleLabel();
  }

private:
  art::ModuleDescription desc_;

  void track_module(art::ModuleDescription const& desc);

}; // CurrentModule

// ======================================================================

DECLARE_ART_SYSTEM_SERVICE(art::CurrentModule, LEGACY)
#endif /* art_Framework_Services_System_CurrentModule_h */

// Local Variables:
// mode: c++
// End:
