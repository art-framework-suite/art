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
  r.sPreModuleConstruction.watch(&CurrentModule::track_module, *this);
  r.sPreModule.watchAll(&CurrentModule::track_module, *this);
  r.sPreModuleBeginJob.watch(&CurrentModule::track_module, *this);
  r.sPreModuleEndJob.watch(&CurrentModule::track_module, *this);
  r.sPreModuleBeginRun.watch(&CurrentModule::track_module, *this);
  r.sPreModuleEndRun.watch(&CurrentModule::track_module, *this);
  r.sPreModuleBeginSubRun.watch(&CurrentModule::track_module, *this);
  r.sPreModuleEndSubRun.watch(&CurrentModule::track_module, *this);
}  // CurrentModule()

// ----------------------------------------------------------------------

void
  CurrentModule::track_module( ModuleDescription const & desc )
{
  desc_ = desc;
}

// ----------------------------------------------------------------------


// ======================================================================
