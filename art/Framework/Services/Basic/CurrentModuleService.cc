// ======================================================================
//
// Track and make available information re the currently-running module
//
// ======================================================================


#include "art/Persistency/Provenance/ModuleDescription.h"
  using edm::ModuleDescription;

#include "art/Framework/Services/Registry/ActivityRegistry.h"
  using edm::ActivityRegistry;

#include "art/Framework/Services/Basic/CurrentModuleService.h"
  using edm::CurrentModuleService;

#include "fhiclcpp/ParameterSet.h"
  using fhicl::ParameterSet;


// ======================================================================


CurrentModuleService::CurrentModuleService( ParameterSet const & //unused
                                          , ActivityRegistry   & r
                                          )
: desc_( )
{
  // activities to monitor in order to note the current module
  r.watchPreModuleConstruction( this, & CurrentModuleService::note_module );
  r.watchPreModule            ( this, & CurrentModuleService::note_module );
  r.watchPreModuleBeginJob    ( this, & CurrentModuleService::note_module );
  r.watchPreModuleEndJob      ( this, & CurrentModuleService::note_module );
  r.watchPreModuleBeginRun    ( this, & CurrentModuleService::note_module );
  r.watchPreModuleEndRun      ( this, & CurrentModuleService::note_module );
  r.watchPreModuleBeginSubRun ( this, & CurrentModuleService::note_module );
  r.watchPreModuleEndSubRun   ( this, & CurrentModuleService::note_module );
}  // CurrentModuleService()


CurrentModuleService::~CurrentModuleService()
{ }

void
  CurrentModuleService::note_module( ModuleDescription const & desc )
{
  desc_ = desc;
}

// ======================================================================
