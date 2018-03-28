#define MFSU_IMPL
#include "art/Framework/Core/MFStatusUpdater.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <sstream>
#include <string>
#include <tuple>
#include <utility>

using namespace std;
using namespace std::string_literals;

#define MFSU_WATCH_UPDATER(cb)                                                 \
  areg.s##cb.watch(this, &MFStatusUpdater::updateStatusTo##cb)

namespace art {

  MFStatusUpdater::~MFStatusUpdater() {}

  MFStatusUpdater::MFStatusUpdater(ActivityRegistry& areg)
  {
    MFSU_WATCH_UPDATER(PostBeginJob);
    MFSU_WATCH_UPDATER(PostEndJob);
    MFSU_WATCH_UPDATER(PostSourceConstruction);
    MFSU_WATCH_UPDATER(PreSourceEvent);
    MFSU_WATCH_UPDATER(PostSourceEvent);
    MFSU_WATCH_UPDATER(PreSourceSubRun);
    MFSU_WATCH_UPDATER(PostSourceSubRun);
    MFSU_WATCH_UPDATER(PreSourceRun);
    MFSU_WATCH_UPDATER(PostSourceRun);
    MFSU_WATCH_UPDATER(PreOpenFile);
    MFSU_WATCH_UPDATER(PostOpenFile);
    MFSU_WATCH_UPDATER(PreCloseFile);
    MFSU_WATCH_UPDATER(PostCloseFile);
    MFSU_WATCH_UPDATER(PreProcessEvent);
    MFSU_WATCH_UPDATER(PostProcessEvent);
    MFSU_WATCH_UPDATER(PreBeginRun);
    MFSU_WATCH_UPDATER(PostBeginRun);
    MFSU_WATCH_UPDATER(PreEndRun);
    MFSU_WATCH_UPDATER(PostEndRun);
    MFSU_WATCH_UPDATER(PreBeginSubRun);
    MFSU_WATCH_UPDATER(PostBeginSubRun);
    MFSU_WATCH_UPDATER(PreEndSubRun);
    MFSU_WATCH_UPDATER(PostEndSubRun);
    MFSU_WATCH_UPDATER(PreProcessPath);
    MFSU_WATCH_UPDATER(PostProcessPath);
    MFSU_WATCH_UPDATER(PrePathBeginRun);
    MFSU_WATCH_UPDATER(PostPathBeginRun);
    MFSU_WATCH_UPDATER(PrePathEndRun);
    MFSU_WATCH_UPDATER(PostPathEndRun);
    MFSU_WATCH_UPDATER(PrePathBeginSubRun);
    MFSU_WATCH_UPDATER(PostPathBeginSubRun);
    MFSU_WATCH_UPDATER(PrePathEndSubRun);
    MFSU_WATCH_UPDATER(PostPathEndSubRun);
    MFSU_WATCH_UPDATER(PreModuleConstruction);
    MFSU_WATCH_UPDATER(PostModuleConstruction);
    MFSU_WATCH_UPDATER(PreModuleBeginJob);
    MFSU_WATCH_UPDATER(PostModuleBeginJob);
    MFSU_WATCH_UPDATER(PreModuleEndJob);
    MFSU_WATCH_UPDATER(PostModuleEndJob);
    MFSU_WATCH_UPDATER(PreModule);
    MFSU_WATCH_UPDATER(PostModule);
    MFSU_WATCH_UPDATER(PreModuleBeginRun);
    MFSU_WATCH_UPDATER(PostModuleBeginRun);
    MFSU_WATCH_UPDATER(PreModuleEndRun);
    MFSU_WATCH_UPDATER(PostModuleEndRun);
    MFSU_WATCH_UPDATER(PreModuleBeginSubRun);
    MFSU_WATCH_UPDATER(PostModuleBeginSubRun);
    MFSU_WATCH_UPDATER(PreModuleEndSubRun);
    MFSU_WATCH_UPDATER(PostModuleEndSubRun);
  }

#undef MFSU_WATCH_UPDATER

  MFSU_0_ARG_UPDATER_DEFN(PostBeginJob)
  {
    mf::SetModuleName("PostBeginJob"s);
    mf::SetIteration("BeforeEvents"s);
  }

  MFSU_0_ARG_UPDATER_DEFN(PostEndJob) { mf::SetModuleName("PostEndJob"s); }

  MFSU_1_ARG_UPDATER_DEFN(PostSourceConstruction)
  {
    mf::SetModuleName("PostSourceConstruction"s);
    mf::SetIteration("SourceConstruction"s);
  }

  MFSU_0_ARG_UPDATER_DEFN(PreSourceEvent) { mf::SetModuleName("SourceEvent"s); }

  MFSU_1_ARG_UPDATER_DEFN(PostSourceEvent)
  {
    mf::SetModuleName("PostSourceEvent"s);
  }

  MFSU_0_ARG_UPDATER_DEFN(PreSourceSubRun)
  {
    mf::SetModuleName("SourceSubRun"s);
  }

  MFSU_1_ARG_UPDATER_DEFN(PostSourceSubRun)
  {
    mf::SetModuleName("PostSourceSubRun"s);
  }

  MFSU_0_ARG_UPDATER_DEFN(PreSourceRun) { mf::SetModuleName("SourceRun"s); }

  MFSU_1_ARG_UPDATER_DEFN(PostSourceRun)
  {
    mf::SetModuleName("PostSourceRun"s);
  }

  MFSU_0_ARG_UPDATER_DEFN(PreOpenFile) { mf::SetModuleName("OpenFile"s); }

  MFSU_1_ARG_UPDATER_DEFN(PostOpenFile) { mf::SetModuleName("PostOpenFile"s); }

  MFSU_0_ARG_UPDATER_DEFN(PreCloseFile) { mf::SetModuleName("CloseFile"s); }

  MFSU_0_ARG_UPDATER_DEFN(PostCloseFile)
  {
    mf::SetModuleName("PostCloseFile"s);
  }

  MFSU_1_ARG_UPDATER_DEFN(PreProcessEvent)
  {
    mf::SetModuleName("ProcessEvent"s);
    std::ostringstream os;
    os << arg1.id();
    mf::SetIteration(os.str());
  }

  MFSU_1_ARG_UPDATER_DEFN(PostProcessEvent)
  {
    mf::SetModuleName("PostProcessEvent"s);
  }

  MFSU_1_ARG_UPDATER_DEFN(PreBeginRun)
  {
    mf::SetModuleName("BeginRun"s);
    std::ostringstream os;
    os << arg1.id();
    mf::SetIteration(os.str());
  }

  MFSU_1_ARG_UPDATER_DEFN(PostBeginRun) { mf::SetModuleName("PostBeginRun"s); }

  MFSU_2_ARG_UPDATER_DEFN(PreEndRun)
  {
    mf::SetModuleName("EndRun"s);
    std::stringstream os;
    os << arg1;
    mf::SetIteration(os.str());
  }

  MFSU_1_ARG_UPDATER_DEFN(PostEndRun) { mf::SetModuleName("PostEndRun"s); }

  MFSU_1_ARG_UPDATER_DEFN(PreBeginSubRun)
  {
    mf::SetModuleName("BeginSubRun"s);
    std::ostringstream os;
    os << arg1.id();
    mf::SetIteration(os.str());
  }

  MFSU_1_ARG_UPDATER_DEFN(PostBeginSubRun)
  {
    mf::SetModuleName("PostBeginSubRun"s);
  }

  MFSU_2_ARG_UPDATER_DEFN(PreEndSubRun)
  {
    mf::SetModuleName("EndSubRun"s);
    std::ostringstream os;
    os << arg1;
    mf::SetIteration(os.str());
  }

  MFSU_1_ARG_UPDATER_DEFN(PostEndSubRun)
  {
    mf::SetModuleName("PostEndSubRun"s);
  }

  MFSU_1_ARG_UPDATER_DEFN(PreProcessPath)
  {
    mf::SetModuleName("ProcessPath "s + arg1);
  }

  MFSU_2_ARG_UPDATER_DEFN(PostProcessPath)
  {
    mf::SetModuleName("PostProcessPath "s + arg1);
  }

  MFSU_1_ARG_UPDATER_DEFN(PrePathBeginRun)
  {
    mf::SetModuleName("PathBeginRun "s + arg1);
  }

  MFSU_2_ARG_UPDATER_DEFN(PostPathBeginRun)
  {
    mf::SetModuleName("PostPathBeginRun "s + arg1);
  }

  MFSU_1_ARG_UPDATER_DEFN(PrePathEndRun)
  {
    mf::SetModuleName("PathEndRun "s + arg1);
  }

  MFSU_2_ARG_UPDATER_DEFN(PostPathEndRun)
  {
    mf::SetModuleName("PostPathEndRun "s + arg1);
  }

  MFSU_1_ARG_UPDATER_DEFN(PrePathBeginSubRun)
  {
    mf::SetModuleName("PathBeginSubRun "s + arg1);
  }

  MFSU_2_ARG_UPDATER_DEFN(PostPathBeginSubRun)
  {
    mf::SetModuleName("PostPathBeginSubRun "s + arg1);
  }

  MFSU_1_ARG_UPDATER_DEFN(PrePathEndSubRun)
  {
    mf::SetModuleName("PathEndSubRun "s + arg1);
  }

  MFSU_2_ARG_UPDATER_DEFN(PostPathEndSubRun)
  {
    mf::SetModuleName("PostPathEndSubRun "s + arg1);
  }

  MFSU_1_ARG_UPDATER_DEFN(PreModuleConstruction)
  {
    mf::SetModuleName(arg1.moduleName() + ":"s + arg1.moduleLabel() +
                      "@Construction"s);
    mf::SetIteration("ModuleConstruction"s);
  }

  MFSU_1_ARG_UPDATER_DEFN(PostModuleConstruction)
  {
    mf::SetModuleName(arg1.moduleName() + ":"s + arg1.moduleLabel() +
                      "@Construction"s);
  }

  MFSU_2_ARG_UPDATER_DEFN(PostBeginJobWorkers)
  {
    throw cet::exception("InternalError"s) << "NOP: do not call";
  }

  MFSU_1_ARG_UPDATER_DEFN(PreModuleBeginJob)
  {
    mf::SetModuleName(arg1.moduleName() + ":"s + arg1.moduleLabel() +
                      "@BeginJob"s);
    mf::SetIteration("ModuleBeginJob"s);
  }

  MFSU_1_ARG_UPDATER_DEFN(PostModuleBeginJob)
  {
    mf::SetModuleName(arg1.moduleName() + ":"s + arg1.moduleLabel() +
                      "@BeginJob"s);
  }

  MFSU_1_ARG_UPDATER_DEFN(PreModuleEndJob)
  {
    mf::SetModuleName(arg1.moduleName() + ":"s + arg1.moduleLabel() +
                      "@EndJob"s);
    mf::SetIteration("ModuleEndJob"s);
  }

  MFSU_1_ARG_UPDATER_DEFN(PostModuleEndJob)
  {
    mf::SetModuleName(arg1.moduleName() + ":"s + arg1.moduleLabel() +
                      "@EndJob"s);
  }

  MFSU_1_ARG_UPDATER_DEFN(PreModule)
  {
    mf::SetModuleName(arg1.moduleName() + ":"s + arg1.moduleLabel() +
                      "@BeginModule"s);
  }

  MFSU_1_ARG_UPDATER_DEFN(PostModule)
  {
    mf::SetModuleName(arg1.moduleName() + ":"s + arg1.moduleLabel() +
                      "@EndModule"s);
  }

  MFSU_1_ARG_UPDATER_DEFN(PreModuleBeginRun)
  {
    mf::SetModuleName(arg1.moduleName() + ":"s + arg1.moduleLabel() +
                      "@BeginRun"s);
  }

  MFSU_1_ARG_UPDATER_DEFN(PostModuleBeginRun)
  {
    mf::SetModuleName(arg1.moduleName() + ":"s + arg1.moduleLabel() +
                      "@BeginRun"s);
  }

  MFSU_1_ARG_UPDATER_DEFN(PreModuleEndRun)
  {
    mf::SetModuleName(arg1.moduleName() + ":"s + arg1.moduleLabel() +
                      "@EndRun"s);
  }

  MFSU_1_ARG_UPDATER_DEFN(PostModuleEndRun)
  {
    mf::SetModuleName(arg1.moduleName() + ":"s + arg1.moduleLabel() +
                      "@EndRun"s);
  }

  MFSU_1_ARG_UPDATER_DEFN(PreModuleBeginSubRun)
  {
    mf::SetModuleName(arg1.moduleName() + ":"s + arg1.moduleLabel() +
                      "@BeginSubRun"s);
  }

  MFSU_1_ARG_UPDATER_DEFN(PostModuleBeginSubRun)
  {
    mf::SetModuleName(arg1.moduleName() + ":"s + arg1.moduleLabel() +
                      "@BeginSubRun"s);
  }

  MFSU_1_ARG_UPDATER_DEFN(PreModuleEndSubRun)
  {
    mf::SetModuleName(arg1.moduleName() + ":"s + arg1.moduleLabel() +
                      "@EndSubRun"s);
  }

  MFSU_1_ARG_UPDATER_DEFN(PostModuleEndSubRun)
  {
    mf::SetModuleName(arg1.moduleName() + ":"s + arg1.moduleLabel() +
                      "@EndSubRun"s);
  }

} // namespace art
