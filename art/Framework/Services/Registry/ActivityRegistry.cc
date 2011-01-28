//
// Package:     ServiceRegistry
// Class  :     ActivityRegistry
//

#define AR_IMPL
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#undef AR_IMPL

#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "cetlib/container_algorithms.h"
#include "messagefacility/MessageLogger/MessageLoggerQ.h"

#include <algorithm>


using namespace cet;
using namespace std;


//
// member functions
//
art::ActivityRegistry::ActivityRegistry()
   :
   programStatus_(),
   workFlowStatus_(),
   md_(*mf::MessageDrop::instance()),
   mls_(*(mf::MessageFacilityService::instance().theML.get()))
 {
   AR_WATCH_UPDATER(PostBeginJob);
   AR_WATCH_UPDATER(PostEndJob);
   AR_WATCH_UPDATER(JobFailure);
   AR_WATCH_UPDATER(PreSource);
   AR_WATCH_UPDATER(PostSource);
   AR_WATCH_UPDATER(PreSourceSubRun);
   AR_WATCH_UPDATER(PostSourceSubRun);
   AR_WATCH_UPDATER(PreSourceRun);
   AR_WATCH_UPDATER(PostSourceRun);
   AR_WATCH_UPDATER(PreOpenFile);
   AR_WATCH_UPDATER(PostOpenFile);
   AR_WATCH_UPDATER(PreCloseFile);
   AR_WATCH_UPDATER(PostCloseFile);
   AR_WATCH_UPDATER(PreProcessEvent);
   AR_WATCH_UPDATER(PostProcessEvent);
   AR_WATCH_UPDATER(PreBeginRun);
   AR_WATCH_UPDATER(PostBeginRun);
   AR_WATCH_UPDATER(PreEndRun);
   AR_WATCH_UPDATER(PostEndRun);
   AR_WATCH_UPDATER(PreBeginSubRun);
   AR_WATCH_UPDATER(PostBeginSubRun);
   AR_WATCH_UPDATER(PreEndSubRun);
   AR_WATCH_UPDATER(PostEndSubRun);
   AR_WATCH_UPDATER(PreProcessPath);
   AR_WATCH_UPDATER(PostProcessPath);
   AR_WATCH_UPDATER(PrePathBeginRun);
   AR_WATCH_UPDATER(PostPathBeginRun);
   AR_WATCH_UPDATER(PrePathEndRun);
   AR_WATCH_UPDATER(PostPathEndRun);
   AR_WATCH_UPDATER(PrePathBeginSubRun);
   AR_WATCH_UPDATER(PostPathBeginSubRun);
   AR_WATCH_UPDATER(PrePathEndSubRun);
   AR_WATCH_UPDATER(PostPathEndSubRun);
   AR_WATCH_UPDATER(PreModuleConstruction);
   AR_WATCH_UPDATER(PostModuleConstruction);
   //   AR_WATCH_UPDATER(PostBeginJobWorkers); // Nothing to do.
   AR_WATCH_UPDATER(PreModuleBeginJob);
   AR_WATCH_UPDATER(PostModuleBeginJob);
   AR_WATCH_UPDATER(PreModuleEndJob);
   AR_WATCH_UPDATER(PostModuleEndJob);
   AR_WATCH_UPDATER(PreModule);
   AR_WATCH_UPDATER(PostModule);
   AR_WATCH_UPDATER(PreModuleBeginRun);
   AR_WATCH_UPDATER(PostModuleBeginRun);
   AR_WATCH_UPDATER(PreModuleEndRun);
   AR_WATCH_UPDATER(PostModuleEndRun);
   AR_WATCH_UPDATER(PreModuleBeginSubRun);
   AR_WATCH_UPDATER(PostModuleBeginSubRun);
   AR_WATCH_UPDATER(PreModuleEndSubRun);
   AR_WATCH_UPDATER(PostModuleEndSubRun);
   AR_WATCH_UPDATER(PreSourceConstruction);
   AR_WATCH_UPDATER(PostSourceConstruction);
}

void
art::ActivityRegistry::connect(ActivityRegistry& iOther)
{
   postBeginJobSignal_.connect(iOther.postBeginJobSignal_);
   postEndJobSignal_.connect(iOther.postEndJobSignal_);

   jobFailureSignal_.connect(iOther.jobFailureSignal_);

   preSourceSignal_.connect(iOther.preSourceSignal_);
   postSourceSignal_.connect(iOther.postSourceSignal_);

   preSourceSubRunSignal_.connect(iOther.preSourceSubRunSignal_);
   postSourceSubRunSignal_.connect(iOther.postSourceSubRunSignal_);

   preSourceRunSignal_.connect(iOther.preSourceRunSignal_);
   postSourceRunSignal_.connect(iOther.postSourceRunSignal_);

   preOpenFileSignal_.connect(iOther.preOpenFileSignal_);
   postOpenFileSignal_.connect(iOther.postOpenFileSignal_);

   preCloseFileSignal_.connect(iOther.preCloseFileSignal_);
   postCloseFileSignal_.connect(iOther.postCloseFileSignal_);

   preProcessEventSignal_.connect(iOther.preProcessEventSignal_);
   postProcessEventSignal_.connect(iOther.postProcessEventSignal_);

   preBeginRunSignal_.connect(iOther.preBeginRunSignal_);
   postBeginRunSignal_.connect(iOther.postBeginRunSignal_);

   preEndRunSignal_.connect(iOther.preEndRunSignal_);
   postEndRunSignal_.connect(iOther.postEndRunSignal_);

   preBeginSubRunSignal_.connect(iOther.preBeginSubRunSignal_);
   postBeginSubRunSignal_.connect(iOther.postBeginSubRunSignal_);

   preEndSubRunSignal_.connect(iOther.preEndSubRunSignal_);
   postEndSubRunSignal_.connect(iOther.postEndSubRunSignal_);

   preProcessPathSignal_.connect(iOther.preProcessPathSignal_);
   postProcessPathSignal_.connect(iOther.postProcessPathSignal_);

   prePathBeginRunSignal_.connect(iOther.prePathBeginRunSignal_);
   postPathBeginRunSignal_.connect(iOther.postPathBeginRunSignal_);

   prePathEndRunSignal_.connect(iOther.prePathEndRunSignal_);
   postPathEndRunSignal_.connect(iOther.postPathEndRunSignal_);

   prePathBeginSubRunSignal_.connect(iOther.prePathBeginSubRunSignal_);
   postPathBeginSubRunSignal_.connect(iOther.postPathBeginSubRunSignal_);

   prePathEndSubRunSignal_.connect(iOther.prePathEndSubRunSignal_);
   postPathEndSubRunSignal_.connect(iOther.postPathEndSubRunSignal_);

   preModuleSignal_.connect(iOther.preModuleSignal_);
   postModuleSignal_.connect(iOther.postModuleSignal_);

   preModuleBeginRunSignal_.connect(iOther.preModuleBeginRunSignal_);
   postModuleBeginRunSignal_.connect(iOther.postModuleBeginRunSignal_);

   preModuleEndRunSignal_.connect(iOther.preModuleEndRunSignal_);
   postModuleEndRunSignal_.connect(iOther.postModuleEndRunSignal_);

   preModuleBeginSubRunSignal_.connect(iOther.preModuleBeginSubRunSignal_);
   postModuleBeginSubRunSignal_.connect(iOther.postModuleBeginSubRunSignal_);

   preModuleEndSubRunSignal_.connect(iOther.preModuleEndSubRunSignal_);
   postModuleEndSubRunSignal_.connect(iOther.postModuleEndSubRunSignal_);

   preModuleConstructionSignal_.connect(iOther.preModuleConstructionSignal_);
   postModuleConstructionSignal_.connect(iOther.postModuleConstructionSignal_);

   postBeginJobWorkersSignal_.connect(iOther.postBeginJobWorkersSignal_);

   preModuleBeginJobSignal_.connect(iOther.preModuleBeginJobSignal_);
   postModuleBeginJobSignal_.connect(iOther.postModuleBeginJobSignal_);

   preModuleEndJobSignal_.connect(iOther.preModuleEndJobSignal_);
   postModuleEndJobSignal_.connect(iOther.postModuleEndJobSignal_);

   preSourceConstructionSignal_.connect(iOther.preSourceConstructionSignal_);
   postSourceConstructionSignal_.connect(iOther.postSourceConstructionSignal_);

}

template<class T>
static
inline
void
copySlotsToFrom(T& iTo, T& iFrom)
{
  typename T::slot_list_type slots = iFrom.slots();

  for_all(slots, boost::bind( &T::connect, iTo, _1) );
}

template<class T>
static
inline
void
copySlotsToFromReverse(T& iTo, T& iFrom)
{
  // This handles service slots that are supposed to be in reverse
  // order of construction. Copying new ones in is a little
  // tricky.  Here is an example of what follows
  // slots in iTo before  4 3 2 1  and copy in slots in iFrom 8 7 6 5
  // reverse both  1 2 3 4  plus 5 6 7 8
  // then do the copy 1 2 3 4 5 6 7 8
  // then reverse back again to get the desired order
  // 8 7 6 5 4 3 2 1

  typename T::slot_list_type slotsFrom = iFrom.slots();
  typename T::slot_list_type slotsTo   = iTo.slots();

  reverse(slotsTo.begin(), slotsTo.end());
  reverse(slotsFrom.begin(), slotsFrom.end());

  for_all(slotsFrom, boost::bind( &T::connect, iTo, _1) );

  reverse(slotsTo.begin(), slotsTo.end());

  // Be nice and put these back in the state they were
  // at the beginning
  reverse(slotsFrom.begin(), slotsFrom.end());
}

void
art::ActivityRegistry::copySlotsFrom(ActivityRegistry& iOther)
{
  copySlotsToFrom(postBeginJobSignal_,iOther.postBeginJobSignal_);
  copySlotsToFromReverse(postEndJobSignal_,iOther.postEndJobSignal_);

  copySlotsToFromReverse(jobFailureSignal_,iOther.jobFailureSignal_);

  copySlotsToFrom(preSourceSignal_,iOther.preSourceSignal_);
  copySlotsToFromReverse(postSourceSignal_,iOther.postSourceSignal_);

  copySlotsToFrom(preSourceSubRunSignal_,iOther.preSourceSubRunSignal_);
  copySlotsToFromReverse(postSourceSubRunSignal_,iOther.postSourceSubRunSignal_);

  copySlotsToFrom(preSourceRunSignal_,iOther.preSourceRunSignal_);
  copySlotsToFromReverse(postSourceRunSignal_,iOther.postSourceRunSignal_);

  copySlotsToFrom(preOpenFileSignal_,iOther.preOpenFileSignal_);
  copySlotsToFromReverse(postOpenFileSignal_,iOther.postOpenFileSignal_);

  copySlotsToFrom(preCloseFileSignal_,iOther.preCloseFileSignal_);
  copySlotsToFromReverse(postCloseFileSignal_,iOther.postCloseFileSignal_);

  copySlotsToFrom(preProcessEventSignal_,iOther.preProcessEventSignal_);
  copySlotsToFromReverse(postProcessEventSignal_,iOther.postProcessEventSignal_);

  copySlotsToFrom(preBeginRunSignal_,iOther.preBeginRunSignal_);
  copySlotsToFromReverse(postBeginRunSignal_,iOther.postBeginRunSignal_);

  copySlotsToFrom(preEndRunSignal_,iOther.preEndRunSignal_);
  copySlotsToFromReverse(postEndRunSignal_,iOther.postEndRunSignal_);

  copySlotsToFrom(preBeginSubRunSignal_,iOther.preBeginSubRunSignal_);
  copySlotsToFromReverse(postBeginSubRunSignal_,iOther.postBeginSubRunSignal_);

  copySlotsToFrom(preEndSubRunSignal_,iOther.preEndSubRunSignal_);
  copySlotsToFromReverse(postEndSubRunSignal_,iOther.postEndSubRunSignal_);

  copySlotsToFrom(preProcessPathSignal_,iOther.preProcessPathSignal_);
  copySlotsToFromReverse(postProcessPathSignal_,iOther.postProcessPathSignal_);

  copySlotsToFrom(prePathBeginRunSignal_,iOther.prePathBeginRunSignal_);
  copySlotsToFromReverse(postPathBeginRunSignal_,iOther.postPathBeginRunSignal_);

  copySlotsToFrom(prePathEndRunSignal_,iOther.prePathEndRunSignal_);
  copySlotsToFromReverse(postPathEndRunSignal_,iOther.postPathEndRunSignal_);

  copySlotsToFrom(prePathBeginSubRunSignal_,iOther.prePathBeginSubRunSignal_);
  copySlotsToFromReverse(postPathBeginSubRunSignal_,iOther.postPathBeginSubRunSignal_);

  copySlotsToFrom(prePathEndSubRunSignal_,iOther.prePathEndSubRunSignal_);
  copySlotsToFromReverse(postPathEndSubRunSignal_,iOther.postPathEndSubRunSignal_);

  copySlotsToFrom(preModuleSignal_,iOther.preModuleSignal_);
  copySlotsToFromReverse(postModuleSignal_,iOther.postModuleSignal_);

  copySlotsToFrom(preModuleBeginRunSignal_,iOther.preModuleBeginRunSignal_);
  copySlotsToFromReverse(postModuleBeginRunSignal_,iOther.postModuleBeginRunSignal_);

  copySlotsToFrom(preModuleEndRunSignal_,iOther.preModuleEndRunSignal_);
  copySlotsToFromReverse(postModuleEndRunSignal_,iOther.postModuleEndRunSignal_);

  copySlotsToFrom(preModuleBeginSubRunSignal_,iOther.preModuleBeginSubRunSignal_);
  copySlotsToFromReverse(postModuleBeginSubRunSignal_,iOther.postModuleBeginSubRunSignal_);

  copySlotsToFrom(preModuleEndSubRunSignal_,iOther.preModuleEndSubRunSignal_);
  copySlotsToFromReverse(postModuleEndSubRunSignal_,iOther.postModuleEndSubRunSignal_);

  copySlotsToFrom(preModuleConstructionSignal_,iOther.preModuleConstructionSignal_);
  copySlotsToFromReverse(postModuleConstructionSignal_,iOther.postModuleConstructionSignal_);

  copySlotsToFrom(preModuleBeginJobSignal_,iOther.preModuleBeginJobSignal_);
  copySlotsToFromReverse(postModuleBeginJobSignal_,iOther.postModuleBeginJobSignal_);

  copySlotsToFrom(preModuleEndJobSignal_,iOther.preModuleEndJobSignal_);
  copySlotsToFromReverse(postModuleEndJobSignal_,iOther.postModuleEndJobSignal_);

  copySlotsToFrom(preSourceConstructionSignal_,iOther.preSourceConstructionSignal_);
  copySlotsToFromReverse(postSourceConstructionSignal_,iOther.postSourceConstructionSignal_);

  copySlotsToFromReverse(postBeginJobWorkersSignal_,iOther.postBeginJobWorkersSignal_);
}

void art::ActivityRegistry::setContext(std::string const &ps) {
   programStatus_ = ps;
   savedEnabledState_ = mls_.setContext(ps);
}

void art::ActivityRegistry::setMinimalContext(std::string const &ps) {
   programStatus_ = ps;
   mls_.setMinimalContext(ps);
}

void art::ActivityRegistry::setContext(art::ModuleDescription const &desc) {
   programStatus_ = moduleIDString(desc);
   savedEnabledState_ = mls_.setContext(programStatus_, desc.moduleLabel());
}

void art::ActivityRegistry::setContext(art::ModuleDescription const &desc,
                                       std::string const &phase) {
   programStatus_ = moduleIDString(desc, phase);
   savedEnabledState_ = mls_.setContext(programStatus_, desc.moduleLabel());
}

void art::ActivityRegistry::restoreContext(art::ModuleDescription const &desc) {
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

void art::ActivityRegistry::restoreContext(art::ModuleDescription const &desc,
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

void art::ActivityRegistry::restoreContext(std::string const &ps) {
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

void art::ActivityRegistry::setWorkFlowStatus(std::string wfs) {
   workFlowStatus_ = wfs;
   md_.runEvent = wfs;
}

std::string
art::ActivityRegistry::moduleIDString(const ModuleDescription &desc) {
   string result = desc.moduleName();
   result += ":";
   result += desc.moduleLabel();
   return result;
}

std::string
art::ActivityRegistry::moduleIDString(const ModuleDescription &desc,
                                       std::string const &suffix) {
   string result = desc.moduleName();
   result += ":";
   result += desc.moduleLabel();
   result += "@";
   result += suffix;
   return result;
}

AR_0_ARG_UPDATER_DEFN(PostBeginJob) {
   setContext("PostBeginJob");
   setWorkFlowStatus("BeforeEvents");
}

AR_0_ARG_UPDATER_DEFN(PostEndJob) {
   mf::MessageLoggerQ::MLqSUM();
}

AR_0_ARG_UPDATER_DEFN(JobFailure) {
   setContext("JobFailure");
   mf::MessageLoggerQ::MLqSUM();
}

AR_0_ARG_UPDATER_DEFN(PreSource) {
   setContext("Source");
}

AR_0_ARG_UPDATER_DEFN(PostSource) {
   restoreContext("PostSource");
}

AR_0_ARG_UPDATER_DEFN(PreSourceSubRun) {
   setContext("SourceSubRun");
}

AR_0_ARG_UPDATER_DEFN(PostSourceSubRun) {
   restoreContext("PostSourceSubRun");   
}

AR_0_ARG_UPDATER_DEFN(PreSourceRun) {
   setContext("SourceRun");
}

AR_0_ARG_UPDATER_DEFN(PostSourceRun) {
   restoreContext("PostSourceRun");
}

AR_0_ARG_UPDATER_DEFN(PreOpenFile) {
   setContext("OpenFile");
}

AR_0_ARG_UPDATER_DEFN(PostOpenFile) {
   restoreContext("PostOpenFile");
}

AR_0_ARG_UPDATER_DEFN(PreCloseFile) {
   setContext("CloseFile");
}

AR_0_ARG_UPDATER_DEFN(PostCloseFile) {
   restoreContext("PostCloseFile");
}

AR_2_ARG_UPDATER_DEFN(PreProcessEvent) {
   std::ostringstream os;
   os << arg1;
   setWorkFlowStatus(os.str());
}

AR_1_ARG_UPDATER_DEFN(PostProcessEvent) {
   setWorkFlowStatus("PostProcessEvent");
}

AR_2_ARG_UPDATER_DEFN(PreBeginRun) {
   std::ostringstream os;
   os << arg1;
   setWorkFlowStatus(os.str());
}

AR_1_ARG_UPDATER_DEFN(PostBeginRun) {
   setWorkFlowStatus("PostBeginRun");
}

AR_2_ARG_UPDATER_DEFN(PreEndRun) {
   std::stringstream os;
   os << "End " << arg1;
   setWorkFlowStatus(os.str());
}

AR_1_ARG_UPDATER_DEFN(PostEndRun) {
   setWorkFlowStatus("PostEndRun");   
}

AR_2_ARG_UPDATER_DEFN(PreBeginSubRun) {
   std::ostringstream os;
   os << arg1;
   setWorkFlowStatus(os.str());   
}

AR_1_ARG_UPDATER_DEFN(PostBeginSubRun) {
   setWorkFlowStatus("PostBeginSubRun");
}

AR_2_ARG_UPDATER_DEFN(PreEndSubRun) {
   std::ostringstream os;
   os << "End Subrun " << arg1;
   setWorkFlowStatus(os.str());      
}

AR_1_ARG_UPDATER_DEFN(PostEndSubRun) {
   setWorkFlowStatus("PostEndSubRun");
}

AR_1_ARG_UPDATER_DEFN(PreProcessPath) {
   string context = "ProcessPath ";
   context += arg1;
   setContext(context);
}

AR_2_ARG_UPDATER_DEFN(PostProcessPath) {
   string context = "PostProcessPath ";
   context += arg1;
   restoreContext(context);
}

AR_1_ARG_UPDATER_DEFN(PrePathBeginRun) {
   string context = "PathBeginRun ";
   context += arg1;
   setMinimalContext(context);
}

AR_2_ARG_UPDATER_DEFN(PostPathBeginRun) {
   string context = "PostPathBeginRun ";
   context += arg1;
   setMinimalContext(context);
}

AR_1_ARG_UPDATER_DEFN(PrePathEndRun) {
   string context = "PathEndRun ";
   context += arg1;
   setMinimalContext(context);
}

AR_2_ARG_UPDATER_DEFN(PostPathEndRun) {
   string context = "PostPathEndRun ";
   context += arg1;
   setMinimalContext(context);   
}

AR_1_ARG_UPDATER_DEFN(PrePathBeginSubRun) {
   string context = "PathBeginSubRun ";
   context += arg1;
   setMinimalContext(context);
}

AR_2_ARG_UPDATER_DEFN(PostPathBeginSubRun) {
   string context = "PostPathBeginSubRun ";
   context += arg1;
   setMinimalContext(context);
}

AR_1_ARG_UPDATER_DEFN(PrePathEndSubRun) {
   string context = "PathEndSubRun ";
   context += arg1;
   setMinimalContext(context);
}

AR_2_ARG_UPDATER_DEFN(PostPathEndSubRun) {
   string context = "PostPathEndSubRun ";
   context += arg1;
   setMinimalContext(context);
}

AR_1_ARG_UPDATER_DEFN(PreModuleConstruction) {
   setContext(arg1, "Construction");
}

AR_1_ARG_UPDATER_DEFN(PostModuleConstruction) {
   restoreContext(arg1, "Construction");
}

AR_2_ARG_UPDATER_DEFN(PostBeginJobWorkers) {
   throw cet::exception("InternalError")
      << "NOP: do not call";
}

AR_1_ARG_UPDATER_DEFN(PreModuleBeginJob) {
   setContext(arg1, "BeginJob");
}

AR_1_ARG_UPDATER_DEFN(PostModuleBeginJob) {
   restoreContext(arg1, "BeginJob");
}

AR_1_ARG_UPDATER_DEFN(PreModuleEndJob) {
   setContext(arg1, "EndJob");
}

AR_1_ARG_UPDATER_DEFN(PostModuleEndJob) {
   restoreContext(arg1, "EndJob");   
}

AR_1_ARG_UPDATER_DEFN(PreModule) {
   setContext(arg1);
}

AR_1_ARG_UPDATER_DEFN(PostModule) {
   restoreContext(arg1);
}

AR_1_ARG_UPDATER_DEFN(PreModuleBeginRun) {
   setContext(arg1, "BeginRun");
}

AR_1_ARG_UPDATER_DEFN(PostModuleBeginRun) {
   restoreContext(arg1, "BeginRun");
}

AR_1_ARG_UPDATER_DEFN(PreModuleEndRun) {
   setContext(arg1, "EndRun");
}

AR_1_ARG_UPDATER_DEFN(PostModuleEndRun) {
   restoreContext(arg1, "EndRun");
}

AR_1_ARG_UPDATER_DEFN(PreModuleBeginSubRun) {
   setContext(arg1, "BeginSubRun");
}

AR_1_ARG_UPDATER_DEFN(PostModuleBeginSubRun) {
   restoreContext(arg1, "BeginSubRun");   
}

AR_1_ARG_UPDATER_DEFN(PreModuleEndSubRun) {
   setContext(arg1, "EndSubRun");
}

AR_1_ARG_UPDATER_DEFN(PostModuleEndSubRun) {
   restoreContext(arg1, "EndSubRun");
}

AR_1_ARG_UPDATER_DEFN(PreSourceConstruction) {
   setContext(arg1, "SourceConstruction");
}

AR_1_ARG_UPDATER_DEFN(PostSourceConstruction) {
   restoreContext(arg1, "PostSourceConstruction");
}
