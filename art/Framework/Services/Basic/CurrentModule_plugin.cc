// ======================================================================
//
// CurrentModule: A Service to track and make available information re
//                the currently-running module
//
// ======================================================================


#include "art/Framework/Services/Basic/CurrentModule.h"

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceMaker.h"
#include "art/Persistency/Provenance/ModuleDescription.h"

#include "fhiclcpp/ParameterSet.h"

using edm::ActivityRegistry;
using edm::CurrentModule;
using edm::CurrentModule;
using edm::ModuleDescription;
using fhicl::ParameterSet;


// ======================================================================


CurrentModule::CurrentModule( ParameterSet const & //unused
                            , ActivityRegistry   & r
                            )
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


CurrentModule::~CurrentModule()
{ }

void
  CurrentModule::track_module( ModuleDescription const & desc )
{
  desc_ = desc;
}


// ======================================================================


DEFINE_FWK_SERVICE(CurrentModule);
