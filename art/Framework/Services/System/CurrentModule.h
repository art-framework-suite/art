#ifndef art_Framework_Services_System_CurrentModule_h
#define art_Framework_Services_System_CurrentModule_h
// vim: set sw=2 expandtab :

//
// A Service to track and make available information about
// the currently-running module
//

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "hep_concurrency/RecursiveMutex.h"

#include <string>

namespace art {

  class ActivityRegistry;

  class CurrentModule {
    // Special Member Functions
  public:
    CurrentModule(art::ActivityRegistry& r);
    CurrentModule(CurrentModule const&) = delete;
    CurrentModule operator=(CurrentModule const&) = delete;
    // API
  public:
    std::string const& label() const;
    // Implementation details
  private:
    void track_module(art::ModuleDescription const& desc);
    // Member Data
  private:
    // Protects all data members.
    mutable hep::concurrency::RecursiveMutex mutex_{
      "art::CurrentModule::mutex_"};
    art::ModuleDescription desc_;
  };

} // namespace art

DECLARE_ART_SYSTEM_SERVICE(art::CurrentModule, LEGACY)

#endif /* art_Framework_Services_System_CurrentModule_h */

// Local Variables:
// mode: c++
// End:
