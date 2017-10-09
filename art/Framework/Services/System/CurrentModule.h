#ifndef art_Framework_Services_System_CurrentModule_h
#define art_Framework_Services_System_CurrentModule_h
// vim: set sw=2 expandtab :

//
// A Service to track and make available information about
// the currently-running module
//

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include <string>

namespace art {

  class ActivityRegistry;

  class CurrentModule {

  public:
    CurrentModule(CurrentModule const&) = delete;

    CurrentModule operator=(CurrentModule const&) = delete;

    CurrentModule(art::ActivityRegistry& r);

  public:
    std::string const&
    label() const
    {
      return desc_.moduleLabel();
    }

  private:
    art::ModuleDescription desc_;

    void track_module(art::ModuleDescription const& desc);
  };

} // namespace art

DECLARE_ART_SYSTEM_SERVICE(art::CurrentModule, LEGACY)

#endif /* art_Framework_Services_System_CurrentModule_h */

// Local Variables:
// mode: c++
// End:
