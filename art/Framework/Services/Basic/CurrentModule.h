#ifndef CurrentModule_CurrentModule_hh
#define CurrentModule_CurrentModule_hh


// ======================================================================
//
// CurrentModule: A Service to track and make available information re
//                the currently-running module
//
// ======================================================================


#include "art/Persistency/Provenance/ModuleDescription.h"
namespace art {
  class ActivityRegistry;
}

#include "fhiclcpp/ParameterSet.h"

#include <string>


// ======================================================================


namespace art {

  class CurrentModule
  {
  public:
    CurrentModule( fhicl::ParameterSet const &
                 , art::ActivityRegistry &
                 );
    ~CurrentModule();

    std::string  label() const  { return desc_.moduleLabel(); }

  private:
    art::ModuleDescription  desc_;
    void track_module( art::ModuleDescription const & desc );

  };  // CurrentModule

}  // namespace art

#endif  // CurrentModule_CurrentModule_hh
