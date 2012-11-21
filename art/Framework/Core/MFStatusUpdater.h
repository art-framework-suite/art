#ifndef art_Framework_Core_MFStatusUpdater_h
#define art_Framework_Core_MFStatusUpdater_h

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "messagefacility/MessageLogger/MessageDrop.h"
#include "messagefacility/MessageService/MessageLogger.h"

#include <string>

#define MFSU_UPDATER_DECL(stateTag)                                   \
  template <typename ...Args>                                         \
  auto                                                                \
  updateStatusTo##stateTag(Args...) ->                                \
  typename decltype(art::ActivityRegistry::s##stateTag)::result_type

#define MFSU_UPDATER_DEFN(stateTag)                                   \
  template <typename ...Args>                                         \
  auto                                                                \
  art::MFStatusUpdater::                                              \
  updateStatusTo##stateTag(Args...args __attribute__((unused))) ->    \
  typename decltype(art::ActivityRegistry::s##stateTag)::result_type

#define MFSU_0_ARG_UPDATER_DECL(stateTag)                               \
  ActivityRegistry::stateTag::slot_type::result_type updateStatusTo##stateTag()
#define MFSU_1_ARG_UPDATER_DECL(stateTag)                               \
  ActivityRegistry::stateTag::slot_type::result_type                    \
  updateStatusTo##stateTag(ActivityRegistry::stateTag::slot_type::arg1_type_)
#define MFSU_2_ARG_UPDATER_DECL(stateTag)                               \
  ActivityRegistry::stateTag::slot_type::result_type                    \
  updateStatusTo##stateTag(ActivityRegistry::stateTag::slot_type::arg1_type_, \
                           ActivityRegistry::stateTag::slot_type::arg2_type_)

namespace art {
  class MFStatusUpdater;
}

class art::MFStatusUpdater {
public:
  MFStatusUpdater(MFStatusUpdater const&) = delete;
  MFStatusUpdater operator=(MFStatusUpdater const&) = delete;

  MFStatusUpdater(ActivityRegistry &areg);

  // Public interface to get state information.
  std::string const &programStatus() const { return programStatus_; }
  std::string const &workFlowSatus() const { return workFlowStatus_; }

private:
  MFSU_UPDATER_DECL(PostBeginJob);
  MFSU_0_ARG_UPDATER_DECL(PostEndJob);
  MFSU_0_ARG_UPDATER_DECL(JobFailure);
  MFSU_0_ARG_UPDATER_DECL(PreSource);
  MFSU_0_ARG_UPDATER_DECL(PostSource);
  MFSU_0_ARG_UPDATER_DECL(PreSourceSubRun);
  MFSU_0_ARG_UPDATER_DECL(PostSourceSubRun);
  MFSU_0_ARG_UPDATER_DECL(PreSourceRun);
  MFSU_0_ARG_UPDATER_DECL(PostSourceRun);
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
  MFSU_UPDATER_DECL(PreEndSubRun);
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

  void setContext(std::string const &ps);
  void setMinimalContext(std::string const &ps);
  void setContext(art::ModuleDescription const &desc);
  void setContext(art::ModuleDescription const &desc,
                  std::string const &phase);
  void restoreContext(std::string const &ps);
  void restoreContext(art::ModuleDescription const &desc);
  void restoreContext(art::ModuleDescription const &desc,
                      std::string const &phase);
  void setWorkFlowStatus(std::string wfs);
  std::string moduleIDString(const ModuleDescription &desc);
  std::string moduleIDString(const ModuleDescription &desc,
                             std::string const &suffix);

  ActivityRegistry &areg_;

  std::string programStatus_;
  std::string workFlowStatus_;

  mf::MessageDrop& md_;
  mf::service::MessageLogger& mls_;
  mf::service::MessageLogger::EnabledState savedEnabledState_;
};

#undef MFSU_UPDATER_DECL
#ifndef MFSU_IMPL
#undef MFSU_UPDATER_DEFN
#endif

#undef MFSU_0_ARG_UPDATER_DECL
#undef MFSU_1_ARG_UPDATER_DECL
#undef MFSU_2_ARG_UPDATER_DECL

#endif /* art_Framework_Core_MFStatusUpdater_h */


// Local Variables:
// mode: c++
// End:
