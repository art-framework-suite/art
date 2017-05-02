#ifndef art_Framework_Core_MFStatusUpdater_h
#define art_Framework_Core_MFStatusUpdater_h

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"

#include "messagefacility/MessageService/MessageDrop.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <string>

#define MFSU_0_ARG_UPDATER_DECL(stateTag)                       \
  typename decltype(ActivityRegistry::s##stateTag)::result_type \
  updateStatusTo##stateTag()
#define MFSU_0_ARG_UPDATER_DEFN(stateTag)                             \
  typename decltype(art::ActivityRegistry::s##stateTag)::result_type  \
  art::MFStatusUpdater::updateStatusTo##stateTag()

#define MFSU_1_ARG_UPDATER_DECL(stateTag)                               \
  typename decltype(ActivityRegistry::s##stateTag)::result_type         \
  updateStatusTo##stateTag(typename decltype(ActivityRegistry::s##stateTag)::slot_type::argument_type)
#define MFSU_1_ARG_UPDATER_DEFN(stateTag)                               \
  typename decltype(art::ActivityRegistry::s##stateTag)::result_type    \
  art::MFStatusUpdater::                                                \
  updateStatusTo##stateTag(typename decltype(ActivityRegistry::s##stateTag)::slot_type::argument_type arg1 [[gnu::unused]])

#define MFSU_2_ARG_UPDATER_DECL(stateTag)                               \
  typename decltype(ActivityRegistry::s##stateTag)::result_type         \
  updateStatusTo##stateTag(typename decltype(ActivityRegistry::s##stateTag)::slot_type::first_argument_type, \
                           typename decltype(ActivityRegistry::s##stateTag)::slot_type::second_argument_type)
#define MFSU_2_ARG_UPDATER_DEFN(stateTag)                               \
  typename decltype(art::ActivityRegistry::s##stateTag)::result_type    \
  art::MFStatusUpdater::                                                \
  updateStatusTo##stateTag(typename decltype(ActivityRegistry::s##stateTag)::slot_type::first_argument_type arg1 [[gnu::unused]], \
                           typename decltype(ActivityRegistry::s##stateTag)::slot_type::second_argument_type arg2 [[gnu::unused]])

namespace art {
  class MFStatusUpdater;
}

class art::MFStatusUpdater {
public:
  MFStatusUpdater(MFStatusUpdater const&) = delete;
  MFStatusUpdater operator=(MFStatusUpdater const&) = delete;

  MFStatusUpdater(ActivityRegistry &areg);

private:
  MFSU_0_ARG_UPDATER_DECL(PostBeginJob);
  MFSU_0_ARG_UPDATER_DECL(PostEndJob);
  MFSU_0_ARG_UPDATER_DECL(JobFailure);
  MFSU_1_ARG_UPDATER_DECL(PostSourceConstruction);
  MFSU_0_ARG_UPDATER_DECL(PreSourceEvent);
  MFSU_1_ARG_UPDATER_DECL(PostSourceEvent);
  MFSU_0_ARG_UPDATER_DECL(PreSourceSubRun);
  MFSU_1_ARG_UPDATER_DECL(PostSourceSubRun);
  MFSU_0_ARG_UPDATER_DECL(PreSourceRun);
  MFSU_1_ARG_UPDATER_DECL(PostSourceRun);
  MFSU_0_ARG_UPDATER_DECL(PreOpenFile);
  MFSU_1_ARG_UPDATER_DECL(PostOpenFile);
  MFSU_0_ARG_UPDATER_DECL(PreCloseFile);
  MFSU_0_ARG_UPDATER_DECL(PostCloseFile);
  MFSU_1_ARG_UPDATER_DECL(PreProcessEvent);
  MFSU_1_ARG_UPDATER_DECL(PostProcessEvent);
  MFSU_1_ARG_UPDATER_DECL(PreBeginRun);
  MFSU_1_ARG_UPDATER_DECL(PostBeginRun);
  MFSU_2_ARG_UPDATER_DECL(PreEndRun);
  MFSU_1_ARG_UPDATER_DECL(PostEndRun);
  MFSU_1_ARG_UPDATER_DECL(PreBeginSubRun);
  MFSU_1_ARG_UPDATER_DECL(PostBeginSubRun);
  MFSU_2_ARG_UPDATER_DECL(PreEndSubRun);
  MFSU_1_ARG_UPDATER_DECL(PostEndSubRun);
  MFSU_1_ARG_UPDATER_DECL(PreProcessPath);
  MFSU_2_ARG_UPDATER_DECL(PostProcessPath);
  MFSU_1_ARG_UPDATER_DECL(PrePathBeginRun);
  MFSU_2_ARG_UPDATER_DECL(PostPathBeginRun);
  MFSU_2_ARG_UPDATER_DECL(PostPathBeginSubRun);
  MFSU_1_ARG_UPDATER_DECL(PrePathEndRun);
  MFSU_1_ARG_UPDATER_DECL(PrePathBeginSubRun);
  MFSU_2_ARG_UPDATER_DECL(PostPathEndRun);
  MFSU_1_ARG_UPDATER_DECL(PrePathEndSubRun);
  MFSU_2_ARG_UPDATER_DECL(PostPathEndSubRun);
  MFSU_1_ARG_UPDATER_DECL(PreModuleConstruction);
  MFSU_1_ARG_UPDATER_DECL(PostModuleConstruction);
  MFSU_2_ARG_UPDATER_DECL(PostBeginJobWorkers);
  MFSU_1_ARG_UPDATER_DECL(PreModuleBeginJob);
  MFSU_1_ARG_UPDATER_DECL(PostModuleBeginJob);
  MFSU_1_ARG_UPDATER_DECL(PreModuleEndJob);
  MFSU_1_ARG_UPDATER_DECL(PostModuleEndJob);
  MFSU_1_ARG_UPDATER_DECL(PreModule);
  MFSU_1_ARG_UPDATER_DECL(PostModule);
  MFSU_1_ARG_UPDATER_DECL(PreModuleBeginRun);
  MFSU_1_ARG_UPDATER_DECL(PostModuleBeginRun);
  MFSU_1_ARG_UPDATER_DECL(PreModuleEndRun);
  MFSU_1_ARG_UPDATER_DECL(PostModuleEndRun);
  MFSU_1_ARG_UPDATER_DECL(PreModuleBeginSubRun);
  MFSU_1_ARG_UPDATER_DECL(PostModuleBeginSubRun);
  MFSU_1_ARG_UPDATER_DECL(PreModuleEndSubRun);
  MFSU_1_ARG_UPDATER_DECL(PostModuleEndSubRun);

  void moduleWithPhase(art::ModuleDescription const & desc,
                       std::string const & phase);
  void preModuleWithPhase(art::ModuleDescription const & desc,
                          std::string const & phase = {});
  void postModuleWithPhase(art::ModuleDescription const & desc,
                           std::string const & phase = {});
  void saveEnabledState(art::ModuleDescription const & desc);
  void saveEnabledState(std::string const & moduleLabel);
  void restoreEnabledState();

  ActivityRegistry &areg_;

  mf::MessageDrop& md_;
  mf::EnabledState savedEnabledState_;
};

inline
void
art::MFStatusUpdater::
moduleWithPhase(art::ModuleDescription const & desc,
                std::string const & phase)
{
  md_.setModuleWithPhase(desc.moduleName(),
                         desc.moduleLabel(),
                         desc.id(),
                         phase);
}

inline
void
art::MFStatusUpdater::
preModuleWithPhase(art::ModuleDescription const & desc,
                   std::string const & phase)
{
  moduleWithPhase(desc, phase);
  saveEnabledState(desc);
}

inline
void
art::MFStatusUpdater::
postModuleWithPhase(art::ModuleDescription const & desc,
                    std::string const & phase)
{
  moduleWithPhase(desc, phase);
  restoreEnabledState();
}

inline
void
art::MFStatusUpdater::
saveEnabledState(art::ModuleDescription const & desc)
{
  saveEnabledState(desc.moduleLabel());
}

inline
void
art::MFStatusUpdater::
saveEnabledState(std::string const & moduleLabel)
{
  savedEnabledState_ = mf::setEnabledState(moduleLabel);
}

#undef MFSU_0_ARG_UPDATER_DECL
#undef MFSU_1_ARG_UPDATER_DECL
#undef MFSU_2_ARG_UPDATER_DECL
#undef MFSU_UPDATER_DECL

#ifndef MFSU_IMPL
#undef MFSU_0_ARG_UPDATER_DEFN
#undef MFSU_1_ARG_UPDATER_DEFN
#undef MFSU_2_ARG_UPDATER_DEFN
#endif


#endif /* art_Framework_Core_MFStatusUpdater_h */


// Local Variables:
// mode: c++
// End:
