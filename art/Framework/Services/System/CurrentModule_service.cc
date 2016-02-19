// ======================================================================
//
// CurrentModule: A Service to track and make available information re
//                the currently-running module
//
// ======================================================================

#include "art/Framework/Services/System/CurrentModule.h"

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"

using art::ActivityRegistry;
using art::CurrentModule;
using art::ModuleDescription;
using fhicl::ParameterSet;

// ----------------------------------------------------------------------

CurrentModule::CurrentModule( ActivityRegistry & r )
: desc_( )
{
  // activities to monitor in order to note the current module
  r.sPreModuleConstruction.watch( this, & CurrentModule::track_module );
  r.sPreModule.watch            ( this, & CurrentModule::track_module );
  r.sPreModuleBeginJob.watch    ( this, & CurrentModule::track_module );
  r.sPreModuleEndJob.watch      ( this, & CurrentModule::track_module );
  r.sPreModuleBeginRun.watch    ( this, & CurrentModule::track_module );
  r.sPreModuleEndRun.watch      ( this, & CurrentModule::track_module );
  r.sPreModuleBeginSubRun.watch ( this, & CurrentModule::track_module );
  r.sPreModuleEndSubRun.watch   ( this, & CurrentModule::track_module );
}  // CurrentModule()

// ----------------------------------------------------------------------

void
  CurrentModule::track_module( ModuleDescription const & desc )
{
  desc_ = desc;
}

// ----------------------------------------------------------------------


// ======================================================================
PROVIDE_FILE_PATH()
// ======================================================================
