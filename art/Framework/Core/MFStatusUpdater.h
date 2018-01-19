#ifndef art_Framework_Core_MFStatusUpdater_h
#define art_Framework_Core_MFStatusUpdater_h
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"

#include "messagefacility/MessageLogger/MessageLogger.h"
#include "messagefacility/MessageService/MessageDrop.h"

#include <string>

#define MFSU_0_ARG_UPDATER_DECL(cb)                                            \
  typename decltype(ActivityRegistry::s##cb)::result_type updateStatusTo##cb()

#define MFSU_0_ARG_UPDATER_DEFN(cb)                                            \
  typename decltype(ActivityRegistry::s##cb)::result_type                      \
    MFStatusUpdater::updateStatusTo##cb()

#define MFSU_1_ARG_UPDATER_DECL(cb)                                            \
  typename decltype(ActivityRegistry::s##cb)::result_type updateStatusTo##cb(  \
    typename decltype(ActivityRegistry::s##cb)::slot_type::argument_type)

#define MFSU_1_ARG_UPDATER_DEFN(cb)                                            \
  typename decltype(ActivityRegistry::s##cb)::result_type                      \
    MFStatusUpdater::updateStatusTo##cb(typename decltype(                     \
      ActivityRegistry::s##cb)::slot_type::argument_type arg1[[gnu::unused]])

#define MFSU_2_ARG_UPDATER_DECL(cb)                                            \
  typename decltype(ActivityRegistry::s##cb)::result_type updateStatusTo##cb(  \
    typename decltype(                                                         \
      ActivityRegistry::s##cb)::slot_type::first_argument_type,                \
    typename decltype(                                                         \
      ActivityRegistry::s##cb)::slot_type::second_argument_type)

#define MFSU_2_ARG_UPDATER_DEFN(cb)                                            \
  typename decltype(ActivityRegistry::s##cb)::result_type                      \
    MFStatusUpdater::updateStatusTo##cb(                                       \
      typename decltype(                                                       \
        ActivityRegistry::s##cb)::slot_type::first_argument_type               \
        arg1[[gnu::unused]],                                                   \
      typename decltype(                                                       \
        ActivityRegistry::s##cb)::slot_type::second_argument_type              \
        arg2[[gnu::unused]])

namespace art {

  class MFStatusUpdater {

  public:
    ~MFStatusUpdater();

    MFStatusUpdater(ActivityRegistry& areg);

    MFStatusUpdater(MFStatusUpdater const&) = delete;

    MFStatusUpdater(MFStatusUpdater&&) = delete;

    MFStatusUpdater& operator=(MFStatusUpdater const&) = delete;

    MFStatusUpdater& operator=(MFStatusUpdater&&) = delete;

  private:
    MFSU_1_ARG_UPDATER_DECL(PreModuleConstruction);
    MFSU_1_ARG_UPDATER_DECL(PostModuleConstruction);

  private:
    MFSU_1_ARG_UPDATER_DECL(PostSourceConstruction);

  private:
    MFSU_0_ARG_UPDATER_DECL(PreOpenFile);
    MFSU_1_ARG_UPDATER_DECL(PostOpenFile);
    MFSU_0_ARG_UPDATER_DECL(PreCloseFile);
    MFSU_0_ARG_UPDATER_DECL(PostCloseFile);

  private:
    MFSU_1_ARG_UPDATER_DECL(PreModuleBeginJob);
    MFSU_1_ARG_UPDATER_DECL(PostModuleBeginJob);
    // FIXME: Throws error!  Remove!!!
    MFSU_2_ARG_UPDATER_DECL(PostBeginJobWorkers);
    MFSU_0_ARG_UPDATER_DECL(PostBeginJob);

  private:
    MFSU_0_ARG_UPDATER_DECL(PreSourceRun);
    MFSU_1_ARG_UPDATER_DECL(PostSourceRun);

    MFSU_1_ARG_UPDATER_DECL(PreBeginRun);
    MFSU_1_ARG_UPDATER_DECL(PrePathBeginRun);
    MFSU_1_ARG_UPDATER_DECL(PreModuleBeginRun);

    MFSU_1_ARG_UPDATER_DECL(PostModuleBeginRun);
    MFSU_2_ARG_UPDATER_DECL(PostPathBeginRun);
    MFSU_1_ARG_UPDATER_DECL(PostBeginRun);

  private:
    MFSU_0_ARG_UPDATER_DECL(PreSourceSubRun);
    MFSU_1_ARG_UPDATER_DECL(PostSourceSubRun);

    MFSU_1_ARG_UPDATER_DECL(PreBeginSubRun);
    MFSU_1_ARG_UPDATER_DECL(PrePathBeginSubRun);
    MFSU_1_ARG_UPDATER_DECL(PreModuleBeginSubRun);

    MFSU_1_ARG_UPDATER_DECL(PostModuleBeginSubRun);
    MFSU_2_ARG_UPDATER_DECL(PostPathBeginSubRun);
    MFSU_1_ARG_UPDATER_DECL(PostBeginSubRun);

  private:
    MFSU_0_ARG_UPDATER_DECL(PreSourceEvent);
    MFSU_1_ARG_UPDATER_DECL(PostSourceEvent);

    MFSU_1_ARG_UPDATER_DECL(PreProcessPath);
    MFSU_2_ARG_UPDATER_DECL(PostProcessPath);

    MFSU_1_ARG_UPDATER_DECL(PreProcessEvent);
    MFSU_1_ARG_UPDATER_DECL(PostProcessEvent);

    MFSU_1_ARG_UPDATER_DECL(PreModule);
    MFSU_1_ARG_UPDATER_DECL(PostModule);

  private:
    MFSU_1_ARG_UPDATER_DECL(PreModuleEndSubRun);
    MFSU_1_ARG_UPDATER_DECL(PostModuleEndSubRun);
    MFSU_1_ARG_UPDATER_DECL(PrePathEndSubRun);
    MFSU_2_ARG_UPDATER_DECL(PostPathEndSubRun);
    MFSU_2_ARG_UPDATER_DECL(PreEndSubRun);
    MFSU_1_ARG_UPDATER_DECL(PostEndSubRun);

  private:
    MFSU_1_ARG_UPDATER_DECL(PreModuleEndRun);
    MFSU_1_ARG_UPDATER_DECL(PostModuleEndRun);
    MFSU_1_ARG_UPDATER_DECL(PrePathEndRun);
    MFSU_2_ARG_UPDATER_DECL(PostPathEndRun);
    MFSU_2_ARG_UPDATER_DECL(PreEndRun);
    MFSU_1_ARG_UPDATER_DECL(PostEndRun);

  private:
    MFSU_1_ARG_UPDATER_DECL(PreModuleEndJob);
    MFSU_1_ARG_UPDATER_DECL(PostModuleEndJob);
    MFSU_0_ARG_UPDATER_DECL(PostEndJob);
    MFSU_0_ARG_UPDATER_DECL(JobFailure);

  private:
    void preModuleWithPhase(ModuleDescription const& desc,
                            std::string const& phase = {});
    void postModuleWithPhase(ModuleDescription const& desc,
                             std::string const& phase = {});
    // void restoreEnabledState();

  private:
    mf::MessageDrop& md_;

    // mf::EnabledState
    // savedEnabledState_;
  };

} // namespace art

#undef MFSU_0_ARG_UPDATER_DECL
#undef MFSU_1_ARG_UPDATER_DECL
#undef MFSU_2_ARG_UPDATER_DECL
#undef MFSU_UPDATER_DECL

#ifndef MFSU_IMPL
#undef MFSU_0_ARG_UPDATER_DEFN
#undef MFSU_1_ARG_UPDATER_DEFN
#undef MFSU_2_ARG_UPDATER_DEFN
#endif // MFSU_IMPL

#endif /* art_Framework_Core_MFStatusUpdater_h */

// Local Variables:
// mode: c++
// End:
