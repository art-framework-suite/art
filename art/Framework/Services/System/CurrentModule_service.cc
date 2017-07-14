#include "art/Framework/Services/System/CurrentModule.h"

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "cetlib/assert_only_one_thread.h"

using art::CurrentModule;

// ----------------------------------------------------------------------

art::CurrentModule::CurrentModule(ActivityRegistry& r)
{
  r.sPreModuleConstruction.watch(this, &CurrentModule::track_module);
  r.sPreModule.watch            (this, &CurrentModule::track_module);
  r.sPreModuleBeginJob.watch    (this, &CurrentModule::track_module);
  r.sPreModuleEndJob.watch      (this, &CurrentModule::track_module);
  r.sPreModuleBeginRun.watch    (this, &CurrentModule::track_module);
  r.sPreModuleEndRun.watch      (this, &CurrentModule::track_module);
  r.sPreModuleBeginSubRun.watch (this, &CurrentModule::track_module);
  r.sPreModuleEndSubRun.watch   (this, &CurrentModule::track_module);
}

// ----------------------------------------------------------------------

void
art::CurrentModule::track_module(ModuleDescription const& desc)
{
  CET_ASSERT_ONLY_ONE_THREAD();
  desc_ = desc;
}

// ======================================================================
CET_PROVIDE_FILE_PATH()
// ======================================================================
