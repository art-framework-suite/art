#define MFSU_IMPL
#include "art/Framework/Core/MFStatusUpdater.h"

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "messagefacility/MessageService/MessageLoggerQ.h"

#include <sstream>
#include <string>
#include <tuple>
#include <utility>

using namespace std::string_literals;

#define MFSU_WATCH_UPDATER(stateTag)                                \
  areg.s##stateTag.watch(this, &art::MFStatusUpdater::updateStatusTo##stateTag)

art::MFStatusUpdater::MFStatusUpdater(ActivityRegistry &areg) :
  areg_(areg),
  md_(*mf::MessageDrop::instance())
{
  MFSU_WATCH_UPDATER(PostBeginJob);
  MFSU_WATCH_UPDATER(PostEndJob);
  MFSU_WATCH_UPDATER(JobFailure);
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
  //   MFSU_WATCH_UPDATER(PostBeginJobWorkers); // Nothing to do.
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

void art::MFStatusUpdater::restoreEnabledState() {
  if (savedEnabledState_.isValid()) {
    mf::restoreEnabledState(savedEnabledState_);
    savedEnabledState_.reset();
  } else {
    throw Exception(errors::LogicError, "INTERNAL ERROR:")
      << "Art attempted to restore an invalid module state to the "
      << "message facility.\n"
      << "Report the bug at https://cdcvs.fnal.gov/redmine/projects/art/issues/new.\n";
  }
}

MFSU_0_ARG_UPDATER_DEFN(PostBeginJob) {
  md_.setSinglet("PostBeginJob"s);
  md_.iteration = "BeforeEvents"s;
}

MFSU_0_ARG_UPDATER_DEFN(PostEndJob) {
  md_.setSinglet("PostEndJob"s);
  mf::MessageLoggerQ::MLqSUM();
}

MFSU_0_ARG_UPDATER_DEFN(JobFailure) {
  md_.setSinglet("JobFailure"s);
  mf::MessageLoggerQ::MLqSUM();
}

MFSU_0_ARG_UPDATER_DEFN(PreSourceEvent) {
  md_.setSinglet("SourceEvent"s);
  saveEnabledState("source"s);
}

MFSU_1_ARG_UPDATER_DEFN(PostSourceEvent) {
  md_.setSinglet("PostSourceEvent"s);
  restoreEnabledState();
}

MFSU_0_ARG_UPDATER_DEFN(PreSourceSubRun) {
  md_.setSinglet("SourceSubRun"s);
  saveEnabledState("source"s);
}

MFSU_1_ARG_UPDATER_DEFN(PostSourceSubRun) {
  md_.setSinglet("PostSourceSubRun"s);
  restoreEnabledState();
}

MFSU_0_ARG_UPDATER_DEFN(PreSourceRun) {
  md_.setSinglet("SourceRun"s);
  saveEnabledState("source"s);
}

MFSU_1_ARG_UPDATER_DEFN(PostSourceRun) {
  md_.setSinglet("PostSourceRun"s);
  restoreEnabledState();
}

MFSU_0_ARG_UPDATER_DEFN(PreOpenFile) {
  md_.setSinglet("OpenFile"s);
}

MFSU_1_ARG_UPDATER_DEFN(PostOpenFile) {
  md_.setSinglet("PostOpenFile"s);
}

MFSU_0_ARG_UPDATER_DEFN(PreCloseFile) {
  md_.setSinglet("CloseFile"s);
}

MFSU_0_ARG_UPDATER_DEFN(PostCloseFile) {
  md_.setSinglet("PostCloseFile"s);
}

MFSU_1_ARG_UPDATER_DEFN(PreProcessEvent) {
  md_.setSinglet("ProcessEvent"s);
  std::ostringstream os;
  os << arg1.id();
  md_.iteration = os.str();
}

MFSU_1_ARG_UPDATER_DEFN(PostProcessEvent) {
  md_.setSinglet("PostProcessEvent"s);
}

MFSU_1_ARG_UPDATER_DEFN(PreBeginRun) {
  md_.setSinglet("BeginRun"s);
  std::ostringstream os;
  os << arg1.id();
  md_.iteration = os.str();
}

MFSU_1_ARG_UPDATER_DEFN(PostBeginRun) {
  md_.setSinglet("PostBeginRun"s);
}

MFSU_2_ARG_UPDATER_DEFN(PreEndRun) {
  md_.setSinglet("EndRun"s);
  std::stringstream os;
  os << arg1;
  md_.iteration = os.str();
}

MFSU_1_ARG_UPDATER_DEFN(PostEndRun) {
  md_.setSinglet("PostEndRun"s);
}

MFSU_1_ARG_UPDATER_DEFN(PreBeginSubRun) {
  md_.setSinglet("BeginSubRun"s);
  std::ostringstream os;
  os << arg1.id();
  md_.iteration = os.str();
}

MFSU_1_ARG_UPDATER_DEFN(PostBeginSubRun) {
  md_.setSinglet("PostBeginSubRun"s);
}

MFSU_2_ARG_UPDATER_DEFN(PreEndSubRun) {
  md_.setSinglet("EndSubRun"s);
  std::ostringstream os;
  os << arg1;
  md_.iteration = os.str();
}

MFSU_1_ARG_UPDATER_DEFN(PostEndSubRun) {
  md_.setSinglet("PostEndSubRun"s);
}

MFSU_1_ARG_UPDATER_DEFN(PreProcessPath) {
  md_.setPath(arg1, "ProcessPath"s);
}

MFSU_2_ARG_UPDATER_DEFN(PostProcessPath) {
  md_.setPath(arg1, "PostProcessPath"s);
}

MFSU_1_ARG_UPDATER_DEFN(PrePathBeginRun) {
  md_.setPath(arg1, "PathBeginRun"s);
}

MFSU_2_ARG_UPDATER_DEFN(PostPathBeginRun) {
  md_.setPath(arg1, "PostPathBeginRun"s);
}

MFSU_1_ARG_UPDATER_DEFN(PrePathEndRun) {
  md_.setPath(arg1, "PathEndRun"s);
}

MFSU_2_ARG_UPDATER_DEFN(PostPathEndRun) {
  md_.setPath(arg1, "PostPathEndRun"s);
}

MFSU_1_ARG_UPDATER_DEFN(PrePathBeginSubRun) {
  md_.setPath(arg1, "PathBeginSubRun"s);
}

MFSU_2_ARG_UPDATER_DEFN(PostPathBeginSubRun) {
  md_.setPath(arg1, "PostPathBeginSubRun"s);
}

MFSU_1_ARG_UPDATER_DEFN(PrePathEndSubRun) {
  md_.setPath(arg1, "PathEndSubRun"s);
}

MFSU_2_ARG_UPDATER_DEFN(PostPathEndSubRun) {
  md_.setPath(arg1, "PostPathEndSubRun"s);
}

MFSU_1_ARG_UPDATER_DEFN(PreModuleConstruction) {
  preModuleWithPhase(arg1, "Construction"s);
  md_.iteration = "ModuleConstruction";
}

MFSU_1_ARG_UPDATER_DEFN(PostModuleConstruction) {
  postModuleWithPhase(arg1, "Construction"s);
}

MFSU_2_ARG_UPDATER_DEFN(PostBeginJobWorkers) {
  throw cet::exception("InternalError"s)
    << "NOP: do not call";
}

MFSU_1_ARG_UPDATER_DEFN(PreModuleBeginJob) {
  preModuleWithPhase(arg1, "BeginJob"s);
  md_.iteration = "ModuleBeginJob"s;
}

MFSU_1_ARG_UPDATER_DEFN(PostModuleBeginJob) {
  postModuleWithPhase(arg1, "BeginJob"s);
}

MFSU_1_ARG_UPDATER_DEFN(PreModuleEndJob) {
  preModuleWithPhase(arg1, "EndJob"s);
  md_.iteration = "ModuleEndJob"s;
}

MFSU_1_ARG_UPDATER_DEFN(PostModuleEndJob) {
  postModuleWithPhase(arg1, "EndJob"s);
}

MFSU_1_ARG_UPDATER_DEFN(PreModule) {
  preModuleWithPhase(arg1);
}

MFSU_1_ARG_UPDATER_DEFN(PostModule) {
  postModuleWithPhase(arg1);
}

MFSU_1_ARG_UPDATER_DEFN(PreModuleBeginRun) {
  preModuleWithPhase(arg1, "BeginRun");
}

MFSU_1_ARG_UPDATER_DEFN(PostModuleBeginRun) {
  postModuleWithPhase(arg1, "BeginRun");
}

MFSU_1_ARG_UPDATER_DEFN(PreModuleEndRun) {
  preModuleWithPhase(arg1, "EndRun");
}

MFSU_1_ARG_UPDATER_DEFN(PostModuleEndRun) {
  postModuleWithPhase(arg1, "EndRun");
}

MFSU_1_ARG_UPDATER_DEFN(PreModuleBeginSubRun) {
  preModuleWithPhase(arg1, "BeginSubRun");
}

MFSU_1_ARG_UPDATER_DEFN(PostModuleBeginSubRun) {
  postModuleWithPhase(arg1, "BeginSubRun");
}

MFSU_1_ARG_UPDATER_DEFN(PreModuleEndSubRun) {
  preModuleWithPhase(arg1, "EndSubRun");
}

MFSU_1_ARG_UPDATER_DEFN(PostModuleEndSubRun) {
  postModuleWithPhase(arg1, "EndSubRun");
}
