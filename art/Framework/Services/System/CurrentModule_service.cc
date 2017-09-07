#include "art/Framework/Services/System/CurrentModule.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"

using art::CurrentModule;

namespace art {

CurrentModule::
CurrentModule(ActivityRegistry& r)
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

void
CurrentModule::
track_module(ModuleDescription const& desc)
{
  desc_ = desc;
}

} // namespace art

CET_PROVIDE_FILE_PATH()
