#ifndef CurrentModuleService_CurrentModuleService_hh
#define CurrentModuleService_CurrentModuleService_hh

// ======================================================================
//
// Track and make available information re the currently-running module
//
//
//
//
//
// ======================================================================


// Framework support:
#include "art/Persistency/Provenance/ModuleDescription.h"
namespace edm {
  class ActivityRegistry;
  class ParameterSet;
}

// C++ support:
#include <string>


// ======================================================================


namespace edm {

  class CurrentModuleService
  {
  public:
    CurrentModuleService( fhicl::ParameterSet const &
                        , edm::ActivityRegistry &
                        );
    ~CurrentModuleService();

    std::string  label() const  { return desc_.moduleLabel(); }

  private:
    edm::ModuleDescription  desc_;
    void note_module( edm::ModuleDescription const & desc );

  };  // CurrentModuleService

}  // namespace edm

#endif  // CurrentModuleService_CurrentModuleService_hh
//
// ======================================================================
