// ======================================================================
//
// CurrentModule: A Service to track and make available information re
//                the currently-running module
//
// ======================================================================

#include "art/Framework/Services/System/CurrentModule.h"

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Persistency/Provenance/ModuleDescription.h"

using art::ActivityRegistry;
using art::CurrentModule;
using art::ModuleDescription;
using fhicl::ParameterSet;

// ----------------------------------------------------------------------

CurrentModule::CurrentModule( ActivityRegistry & r )
: desc_( )
{
  // activities to monitor in order to note the current module
  r.watchPreModuleConstruction( this, & CurrentModule::track_module );
  r.watchPreModule            ( this, & CurrentModule::track_module );
  r.watchPreModuleBeginJob    ( this, & CurrentModule::track_module );
  r.watchPreModuleEndJob      ( this, & CurrentModule::track_module );
  r.watchPreModuleBeginRun    ( this, & CurrentModule::track_module );
  r.watchPreModuleEndRun      ( this, & CurrentModule::track_module );
  r.watchPreModuleBeginSubRun ( this, & CurrentModule::track_module );
  r.watchPreModuleEndSubRun   ( this, & CurrentModule::track_module );
}  // CurrentModule()

// ----------------------------------------------------------------------

void
  CurrentModule::track_module( ModuleDescription const & desc )
{
  desc_ = desc;
}

// ----------------------------------------------------------------------


// ======================================================================
