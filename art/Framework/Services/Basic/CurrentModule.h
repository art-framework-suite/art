#ifndef CurrentModule_CurrentModule_hh
#define CurrentModule_CurrentModule_hh


// ======================================================================
//
// CurrentModule: A Service to track and make available information re
//                the currently-running module
//
// ======================================================================


#include "art/Persistency/Provenance/ModuleDescription.h"
namespace edm {
  class ActivityRegistry;
}

#include "fhiclcpp/ParameterSet.h"

#include <string>


// ======================================================================


namespace edm {

  class CurrentModule
  {
  public:
    CurrentModule( fhicl::ParameterSet const &
                 , edm::ActivityRegistry &
                 );
    ~CurrentModule();

    std::string  label() const  { return desc_.moduleLabel(); }

  private:
    edm::ModuleDescription  desc_;
    void track_module( edm::ModuleDescription const & desc );

  };  // CurrentModule

}  // namespace edm

#endif  // CurrentModule_CurrentModule_hh
