#define MFSU_IMPL
#include "art/Framework/Core/MFStatusUpdater.h"

#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "messagefacility/MessageLogger/MessageLoggerQ.h"

#include <sstream>
#include <string>
#include <tuple>
#include <utility>

#define MFSU_WATCH_UPDATER(stateTag)                                \
  areg.s##stateTag.watch(this, &art::MFStatusUpdater::updateStatusTo##stateTag)

#pragma GCC diagnostic ignored "-Wunused-parameter"

art::MFStatusUpdater::MFStatusUpdater(ActivityRegistry &areg) :
  areg_(areg),
  programStatus_(),
  workFlowStatus_(),
  md_(*mf::MessageDrop::instance()),
  mls_(*(mf::MessageFacilityService::instance().theML.get()))
{
  MFSU_WATCH_UPDATER(PostBeginJob);
  MFSU_WATCH_UPDATER(PostEndJob);
  MFSU_WATCH_UPDATER(JobFailure);
  MFSU_WATCH_UPDATER(PreSource);
  MFSU_WATCH_UPDATER(PostSource);
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

void art::MFStatusUpdater::setContext(std::string const &ps) {
  programStatus_ = ps;
  savedEnabledState_ = mls_.setContext(ps);
}

void art::MFStatusUpdater::setMinimalContext(std::string const &ps) {
  programStatus_ = ps;
  mls_.setMinimalContext(ps);
}

void art::MFStatusUpdater::setContext(art::ModuleDescription const &desc) {
  programStatus_ = moduleIDString(desc);
  savedEnabledState_ = mls_.setContext(programStatus_, desc.moduleLabel());
}

void art::MFStatusUpdater::setContext(art::ModuleDescription const &desc,
                                      std::string const &phase) {
  programStatus_ = moduleIDString(desc, phase);
  savedEnabledState_ = mls_.setContext(programStatus_, desc.moduleLabel());
}

void art::MFStatusUpdater::restoreContext(art::ModuleDescription const &desc) {
  programStatus_ = moduleIDString(desc);
  if (savedEnabledState_.isValid()) {
    mls_.setContext(programStatus_, savedEnabledState_);
    savedEnabledState_.reset();
  } else {
    // When we have a full set of watchpoints, we should probably throw
    // here.
    savedEnabledState_ = mls_.setContext(programStatus_);
  }
}

void art::MFStatusUpdater::restoreContext(art::ModuleDescription const &desc,
                                          std::string const &phase) {
  programStatus_ = moduleIDString(desc, phase);
  if (savedEnabledState_.isValid()) {
    mls_.setContext(programStatus_, savedEnabledState_);
    savedEnabledState_.reset();
  } else {
    // When we have a full set of watchpoints, we should probably throw
    // here.
    savedEnabledState_ = mls_.setContext(programStatus_);
  }
}

void art::MFStatusUpdater::restoreContext(std::string const &ps) {
  programStatus_ = ps;
  if (savedEnabledState_.isValid()) {
    mls_.setContext(ps, savedEnabledState_);
    savedEnabledState_.reset();
  } else {
    // When we have a full set of watchpoints, we should probably throw
    // here.
    savedEnabledState_ = mls_.setContext(ps);
  }
}

void art::MFStatusUpdater::setWorkFlowStatus(std::string wfs) {
  workFlowStatus_ = wfs;
  md_.runEvent = wfs;
}

std::string
art::MFStatusUpdater::moduleIDString(const ModuleDescription &desc) {
  std::string result = desc.moduleName();
  result += ":";
  result += desc.moduleLabel();
  return result;
}

std::string
art::MFStatusUpdater::moduleIDString(const ModuleDescription &desc,
                                     std::string const &suffix) {
  std::string result = desc.moduleName();
  result += ":";
  result += desc.moduleLabel();
  result += "@";
  result += suffix;
  return result;
}

MFSU_0_ARG_UPDATER_DEFN(PostBeginJob) {
  setContext("PostBeginJob");
  setWorkFlowStatus("BeforeEvents");
}

MFSU_0_ARG_UPDATER_DEFN(PostEndJob) {
  mf::MessageLoggerQ::MLqSUM();
}

MFSU_0_ARG_UPDATER_DEFN(JobFailure) {
  setContext("JobFailure");
  mf::MessageLoggerQ::MLqSUM();
}

MFSU_0_ARG_UPDATER_DEFN(PreSource) {
  setContext("Source");
}

MFSU_0_ARG_UPDATER_DEFN(PostSource) {
  restoreContext("PostSource");
}

MFSU_0_ARG_UPDATER_DEFN(PreSourceSubRun) {
  setContext("SourceSubRun");
}

MFSU_0_ARG_UPDATER_DEFN(PostSourceSubRun) {
  restoreContext("PostSourceSubRun");
}

MFSU_0_ARG_UPDATER_DEFN(PreSourceRun) {
  setContext("SourceRun");
}

MFSU_0_ARG_UPDATER_DEFN(PostSourceRun) {
  restoreContext("PostSourceRun");
}

MFSU_0_ARG_UPDATER_DEFN(PreOpenFile) {
  setContext("OpenFile");
}

MFSU_1_ARG_UPDATER_DEFN(PostOpenFile) {
  restoreContext("PostOpenFile");
}

MFSU_0_ARG_UPDATER_DEFN(PreCloseFile) {
  setContext("CloseFile");
}

MFSU_0_ARG_UPDATER_DEFN(PostCloseFile) {
  restoreContext("PostCloseFile");
}

MFSU_1_ARG_UPDATER_DEFN(PreProcessEvent) {
  std::ostringstream os;
  os << arg1.id();
  setWorkFlowStatus(os.str());
}

MFSU_1_ARG_UPDATER_DEFN(PostProcessEvent) {
  setWorkFlowStatus("PostProcessEvent");
}

MFSU_1_ARG_UPDATER_DEFN(PreBeginRun) {
  std::ostringstream os;
  os << arg1.id();
  setWorkFlowStatus(os.str());
}

MFSU_1_ARG_UPDATER_DEFN(PostBeginRun) {
  setWorkFlowStatus("PostBeginRun");
}

MFSU_2_ARG_UPDATER_DEFN(PreEndRun) {
  std::stringstream os;
  os << "End " << arg1;
  setWorkFlowStatus(os.str());
}

MFSU_1_ARG_UPDATER_DEFN(PostEndRun) {
  setWorkFlowStatus("PostEndRun");
}

MFSU_1_ARG_UPDATER_DEFN(PreBeginSubRun) {
  std::ostringstream os;
  os << arg1.id();
  setWorkFlowStatus(os.str());
}

MFSU_1_ARG_UPDATER_DEFN(PostBeginSubRun) {
  setWorkFlowStatus("PostBeginSubRun");
}

MFSU_2_ARG_UPDATER_DEFN(PreEndSubRun) {
  std::ostringstream os;
  os << "End Subrun " << arg1;
  setWorkFlowStatus(os.str());
}

MFSU_1_ARG_UPDATER_DEFN(PostEndSubRun) {
  setWorkFlowStatus("PostEndSubRun");
}

MFSU_1_ARG_UPDATER_DEFN(PreProcessPath) {
  std::string context = "ProcessPath ";
  context += arg1;
  setContext(context);
}

MFSU_2_ARG_UPDATER_DEFN(PostProcessPath) {
  std::string context = "PostProcessPath ";
  context += arg1;
  restoreContext(context);
}

MFSU_1_ARG_UPDATER_DEFN(PrePathBeginRun) {
  std::string context = "PathBeginRun ";
  context += arg1;
  setMinimalContext(context);
}

MFSU_2_ARG_UPDATER_DEFN(PostPathBeginRun) {
  std::string context = "PostPathBeginRun ";
  context += arg1;
  setMinimalContext(context);
}

MFSU_1_ARG_UPDATER_DEFN(PrePathEndRun) {
  std::string context = "PathEndRun ";
  context += arg1;
  setMinimalContext(context);
}

MFSU_2_ARG_UPDATER_DEFN(PostPathEndRun) {
  std::string context = "PostPathEndRun ";
  context += arg1;
  setMinimalContext(context);
}

MFSU_1_ARG_UPDATER_DEFN(PrePathBeginSubRun) {
  std::string context = "PathBeginSubRun ";
  context += arg1;
  setMinimalContext(context);
}

MFSU_2_ARG_UPDATER_DEFN(PostPathBeginSubRun) {
  std::string context = "PostPathBeginSubRun ";
  context += arg1;
  setMinimalContext(context);
}

MFSU_1_ARG_UPDATER_DEFN(PrePathEndSubRun) {
  std::string context = "PathEndSubRun ";
  context += arg1;
  setMinimalContext(context);
}

MFSU_2_ARG_UPDATER_DEFN(PostPathEndSubRun) {
  std::string context = "PostPathEndSubRun ";
  context += arg1;
  setMinimalContext(context);
}

MFSU_1_ARG_UPDATER_DEFN(PreModuleConstruction) {
  setContext(arg1, "Construction");
  setWorkFlowStatus("ModuleConstruction");
}

MFSU_1_ARG_UPDATER_DEFN(PostModuleConstruction) {
  restoreContext(arg1, "Construction");
}

MFSU_2_ARG_UPDATER_DEFN(PostBeginJobWorkers) {
  throw cet::exception("InternalError")
    << "NOP: do not call";
}

MFSU_1_ARG_UPDATER_DEFN(PreModuleBeginJob) {
  setContext(arg1, "BeginJob");
}

MFSU_1_ARG_UPDATER_DEFN(PostModuleBeginJob) {
  restoreContext(arg1, "BeginJob");
}

MFSU_1_ARG_UPDATER_DEFN(PreModuleEndJob) {
  setContext(arg1, "EndJob");
}

MFSU_1_ARG_UPDATER_DEFN(PostModuleEndJob) {
  restoreContext(arg1, "EndJob");
}

MFSU_1_ARG_UPDATER_DEFN(PreModule) {
  setContext(arg1);
}

MFSU_1_ARG_UPDATER_DEFN(PostModule) {
  restoreContext(arg1);
}

MFSU_1_ARG_UPDATER_DEFN(PreModuleBeginRun) {
  setContext(arg1, "BeginRun");
}

MFSU_1_ARG_UPDATER_DEFN(PostModuleBeginRun) {
  restoreContext(arg1, "BeginRun");
}

MFSU_1_ARG_UPDATER_DEFN(PreModuleEndRun) {
  setContext(arg1, "EndRun");
}

MFSU_1_ARG_UPDATER_DEFN(PostModuleEndRun) {
  restoreContext(arg1, "EndRun");
}

MFSU_1_ARG_UPDATER_DEFN(PreModuleBeginSubRun) {
  setContext(arg1, "BeginSubRun");
}

MFSU_1_ARG_UPDATER_DEFN(PostModuleBeginSubRun) {
  restoreContext(arg1, "BeginSubRun");
}

MFSU_1_ARG_UPDATER_DEFN(PreModuleEndSubRun) {
  setContext(arg1, "EndSubRun");
}

MFSU_1_ARG_UPDATER_DEFN(PostModuleEndSubRun) {
  restoreContext(arg1, "EndSubRun");
}
