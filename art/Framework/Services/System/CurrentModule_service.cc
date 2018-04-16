#include "art/Framework/Services/System/CurrentModule.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "hep_concurrency/RecursiveMutex.h"

#include <string>

using namespace std;
using namespace hep::concurrency;

namespace art {

  CurrentModule::CurrentModule(ActivityRegistry& r)
  {
    r.sPreModuleConstruction.watch(this, &CurrentModule::track_module);
    r.sPreModule.watch(this, &CurrentModule::track_module);
    r.sPreModuleBeginJob.watch(this, &CurrentModule::track_module);
    r.sPreModuleEndJob.watch(this, &CurrentModule::track_module);
    r.sPreModuleBeginRun.watch(this, &CurrentModule::track_module);
    r.sPreModuleEndRun.watch(this, &CurrentModule::track_module);
    r.sPreModuleBeginSubRun.watch(this, &CurrentModule::track_module);
    r.sPreModuleEndSubRun.watch(this, &CurrentModule::track_module);
  }

  string const&
  label() const
  {
    RecursiveMutexSentry sentry(mutex_, __func__);
    return desc_.moduleLabel();
  }

  void
  CurrentModule::track_module(ModuleDescription const& desc)
  {
    RecursiveMutexSentry sentry(mutex_, __func__);
    desc_ = desc;
  }

} // namespace art

CET_PROVIDE_FILE_PATH()
