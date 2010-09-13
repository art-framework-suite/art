//
// Package:     Services
// Class  :     Tracer
//


#include "art/Framework/Services/Basic/Tracer.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "art/Persistency/Provenance/Timestamp.h"

#include <iostream>

using namespace edm::service;
using fhicl::ParameterSet;


//
// constructors and destructor
//
Tracer::Tracer(ParameterSet const& iPS, ActivityRegistry&iRegistry) :
  indention_(iPS.getString("indention","++")),
  depth_(0)
{
   iRegistry.watchPostBeginJob(this, &Tracer::postBeginJob);
   iRegistry.watchPostEndJob(this, &Tracer::postEndJob);

   iRegistry.watchPreModule(this, &Tracer::preModuleEvent);
   iRegistry.watchPostModule(this, &Tracer::postModuleEvent);

   iRegistry.watchPreSourceConstruction(this, &Tracer::preSourceConstruction);
   iRegistry.watchPostSourceConstruction(this, &Tracer::postSourceConstruction);

   iRegistry.watchPreModuleConstruction(this, &Tracer::preModuleConstruction);
   iRegistry.watchPostModuleConstruction(this, &Tracer::postModuleConstruction);

   iRegistry.watchPreModuleBeginJob(this, &Tracer::preModuleBeginJob);
   iRegistry.watchPostModuleBeginJob(this, &Tracer::postModuleBeginJob);

   iRegistry.watchPreModuleEndJob(this, &Tracer::preModuleEndJob);
   iRegistry.watchPostModuleEndJob(this, &Tracer::postModuleEndJob);

   iRegistry.watchPreModuleBeginRun(this, &Tracer::preModuleBeginRun);
   iRegistry.watchPostModuleBeginRun(this, &Tracer::postModuleBeginRun);

   iRegistry.watchPreModuleEndRun(this, &Tracer::preModuleEndRun);
   iRegistry.watchPostModuleEndRun(this, &Tracer::postModuleEndRun);

   iRegistry.watchPreModuleBeginSubRun(this, &Tracer::preModuleBeginSubRun);
   iRegistry.watchPostModuleBeginSubRun(this, &Tracer::postModuleBeginSubRun);

   iRegistry.watchPreModuleEndSubRun(this, &Tracer::preModuleEndSubRun);
   iRegistry.watchPostModuleEndSubRun(this, &Tracer::postModuleEndSubRun);

   iRegistry.watchPreProcessPath(this, &Tracer::prePathEvent);
   iRegistry.watchPostProcessPath(this, &Tracer::postPathEvent);

   iRegistry.watchPrePathBeginRun(this, &Tracer::prePathBeginRun);
   iRegistry.watchPostPathBeginRun(this, &Tracer::postPathBeginRun);

   iRegistry.watchPrePathEndRun(this, &Tracer::prePathEndRun);
   iRegistry.watchPostPathEndRun(this, &Tracer::postPathEndRun);

   iRegistry.watchPrePathBeginSubRun(this, &Tracer::prePathBeginSubRun);
   iRegistry.watchPostPathBeginSubRun(this, &Tracer::postPathBeginSubRun);

   iRegistry.watchPrePathEndSubRun(this, &Tracer::prePathEndSubRun);
   iRegistry.watchPostPathEndSubRun(this, &Tracer::postPathEndSubRun);

   iRegistry.watchPreProcessEvent(this, &Tracer::preEvent);
   iRegistry.watchPostProcessEvent(this, &Tracer::postEvent);

   iRegistry.watchPreBeginRun(this, &Tracer::preBeginRun);
   iRegistry.watchPostBeginRun(this, &Tracer::postBeginRun);

   iRegistry.watchPreEndRun(this, &Tracer::preEndRun);
   iRegistry.watchPostEndRun(this, &Tracer::postEndRun);

   iRegistry.watchPreBeginSubRun(this, &Tracer::preBeginSubRun);
   iRegistry.watchPostBeginSubRun(this, &Tracer::postBeginSubRun);

   iRegistry.watchPreEndSubRun(this, &Tracer::preEndSubRun);
   iRegistry.watchPostEndSubRun(this, &Tracer::postEndSubRun);

   iRegistry.watchPreSource(this, &Tracer::preSourceEvent);
   iRegistry.watchPostSource(this, &Tracer::postSourceEvent);

   iRegistry.watchPreOpenFile(this, &Tracer::preOpenFile);
   iRegistry.watchPostOpenFile(this, &Tracer::postOpenFile);

   iRegistry.watchPreCloseFile(this, &Tracer::preCloseFile);
   iRegistry.watchPostCloseFile(this, &Tracer::postCloseFile);

   iRegistry.watchPreSourceRun(this, &Tracer::preSourceRun);
   iRegistry.watchPostSourceRun(this, &Tracer::postSourceRun);

   iRegistry.watchPreSourceSubRun(this, &Tracer::preSourceSubRun);
   iRegistry.watchPostSourceSubRun(this, &Tracer::postSourceSubRun);

}

// Tracer::Tracer(Tracer const& rhs)
// {
//    // do actual copying here;
// }

//Tracer::~Tracer()
//{
//}

//
// assignment operators
//
// Tracer const& Tracer::operator=(Tracer const& rhs)
// {
//   //An exception safe implementation is
//   Tracer temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//
void
Tracer::postBeginJob() {
   std::cout <<indention_<<" Job started"<<std::endl;
}
void
Tracer::postEndJob() {
   std::cout <<indention_<<" Job ended"<<std::endl;
}

void
Tracer::preSourceEvent() {
  std::cout <<indention_<<indention_<<"source event"<<std::endl;
}
void
Tracer::postSourceEvent () {
  std::cout <<indention_<<indention_<<"finished: source event"<<std::endl;
}

void
Tracer::preSourceSubRun() {
  std::cout <<indention_<<indention_<<"source subRun"<<std::endl;
}
void
Tracer::postSourceSubRun () {
  std::cout <<indention_<<indention_<<"finished: source subRun"<<std::endl;
}

void
Tracer::preSourceRun() {
  std::cout <<indention_<<indention_<<"source run"<<std::endl;
}
void
Tracer::postSourceRun () {
  std::cout <<indention_<<indention_<<"finished: source run"<<std::endl;
}

void
Tracer::preOpenFile() {
  std::cout <<indention_<<indention_<<"open input file"<<std::endl;
}
void
Tracer::postOpenFile () {
  std::cout <<indention_<<indention_<<"finished: open input file"<<std::endl;
}

void
Tracer::preCloseFile() {
  std::cout <<indention_<<indention_<<"close input file"<<std::endl;
}
void
Tracer::postCloseFile () {
  std::cout <<indention_<<indention_<<"finished: close input file"<<std::endl;
}

void
Tracer::preEvent(EventID const& iID, Timestamp const& iTime) {
   depth_=0;
   std::cout <<indention_<<indention_<<" processing event:"<< iID<<" time:"<<iTime.value()<< std::endl;
}
void
Tracer::postEvent(Event const&) {
   std::cout <<indention_<<indention_<<" finished event:"<<std::endl;
}

void
Tracer::prePathEvent(std::string const& iName) {
  std::cout <<indention_<<indention_<<indention_<<" processing path for event:"<<iName<<std::endl;
}
void
Tracer::postPathEvent(std::string const& iName, HLTPathStatus const&) {
  std::cout <<indention_<<indention_<<indention_<<" finished path for event:"<<std::endl;
}

void
Tracer::preModuleEvent(ModuleDescription const& iDescription) {
   ++depth_;
   std::cout <<indention_<<indention_<<indention_;
   for(unsigned int depth = 0; depth !=depth_; ++depth) {
      std::cout<<indention_;
   }
   std::cout<<" module for event:" <<iDescription.moduleLabel_<<std::endl;
}
void
Tracer::postModuleEvent(ModuleDescription const& iDescription) {
   --depth_;
   std::cout <<indention_<<indention_<<indention_<<indention_;
   for(unsigned int depth = 0; depth !=depth_; ++depth) {
      std::cout<<indention_;
   }

   std::cout<<" finished for event:"<<iDescription.moduleLabel_<<std::endl;
}

void
Tracer::preBeginRun(RunID const& iID, Timestamp const& iTime) {
   depth_=0;
   std::cout <<indention_<<indention_<<" processing begin run:"<< iID<<" time:"<<iTime.value()<< std::endl;
}
void
Tracer::postBeginRun(Run const&) {
   std::cout <<indention_<<indention_<<" finished begin run:"<<std::endl;
}

void
Tracer::prePathBeginRun(std::string const& iName) {
  std::cout <<indention_<<indention_<<indention_<<" processing path for begin run:"<<iName<<std::endl;
}
void
Tracer::postPathBeginRun(std::string const& iName, HLTPathStatus const&) {
  std::cout <<indention_<<indention_<<indention_<<" finished path for begin run:"<<std::endl;
}

void
Tracer::preModuleBeginRun(ModuleDescription const& iDescription) {
   ++depth_;
   std::cout <<indention_<<indention_<<indention_;
   for(unsigned int depth = 0; depth !=depth_; ++depth) {
      std::cout<<indention_;
   }
   std::cout<<" module for begin run:" <<iDescription.moduleLabel_<<std::endl;
}
void
Tracer::postModuleBeginRun(ModuleDescription const& iDescription) {
   --depth_;
   std::cout <<indention_<<indention_<<indention_<<indention_;
   for(unsigned int depth = 0; depth !=depth_; ++depth) {
      std::cout<<indention_;
   }

   std::cout<<" finished for begin run:"<<iDescription.moduleLabel_<<std::endl;
}

void
Tracer::preEndRun(RunID const& iID, Timestamp const& iTime) {
   depth_=0;
   std::cout <<indention_<<indention_<<" processing end run:"<< iID<<" time:"<<iTime.value()<< std::endl;
}
void
Tracer::postEndRun(Run const&) {
   std::cout <<indention_<<indention_<<" finished end run:"<<std::endl;
}

void
Tracer::prePathEndRun(std::string const& iName) {
  std::cout <<indention_<<indention_<<indention_<<" processing path for end run:"<<iName<<std::endl;
}
void
Tracer::postPathEndRun(std::string const& iName, HLTPathStatus const&) {
  std::cout <<indention_<<indention_<<indention_<<" finished path for end run:"<<std::endl;
}

void
Tracer::preModuleEndRun(ModuleDescription const& iDescription) {
   ++depth_;
   std::cout <<indention_<<indention_<<indention_;
   for(unsigned int depth = 0; depth !=depth_; ++depth) {
      std::cout<<indention_;
   }
   std::cout<<" module for end run:" <<iDescription.moduleLabel_<<std::endl;
}
void
Tracer::postModuleEndRun(ModuleDescription const& iDescription) {
   --depth_;
   std::cout <<indention_<<indention_<<indention_<<indention_;
   for(unsigned int depth = 0; depth !=depth_; ++depth) {
      std::cout<<indention_;
   }

   std::cout<<" finished for end run:"<<iDescription.moduleLabel_<<std::endl;
}

void
Tracer::preBeginSubRun(SubRunID const& iID, Timestamp const& iTime) {
   depth_=0;
   std::cout <<indention_<<indention_<<" processing begin subRun:"<< iID<<" time:"<<iTime.value()<< std::endl;
}
void
Tracer::postBeginSubRun(SubRun const&) {
   std::cout <<indention_<<indention_<<" finished begin subRun:"<<std::endl;
}

void
Tracer::prePathBeginSubRun(std::string const& iName) {
  std::cout <<indention_<<indention_<<indention_<<" processing path for begin subRun:"<<iName<<std::endl;
}
void
Tracer::postPathBeginSubRun(std::string const& iName, HLTPathStatus const&) {
  std::cout <<indention_<<indention_<<indention_<<" finished path for begin subRun:"<<std::endl;
}

void
Tracer::preModuleBeginSubRun(ModuleDescription const& iDescription) {
   ++depth_;
   std::cout <<indention_<<indention_<<indention_;
   for(unsigned int depth = 0; depth !=depth_; ++depth) {
      std::cout<<indention_;
   }
   std::cout<<" module for begin subRun:" <<iDescription.moduleLabel_<<std::endl;
}
void
Tracer::postModuleBeginSubRun(ModuleDescription const& iDescription) {
   --depth_;
   std::cout <<indention_<<indention_<<indention_<<indention_;
   for(unsigned int depth = 0; depth !=depth_; ++depth) {
      std::cout<<indention_;
   }

   std::cout<<" finished for begin subRun:"<<iDescription.moduleLabel_<<std::endl;
}

void
Tracer::preEndSubRun(SubRunID const& iID, Timestamp const& iTime) {
   depth_=0;
   std::cout <<indention_<<indention_<<" processing end subRun:"<< iID<<" time:"<<iTime.value()<< std::endl;
}
void
Tracer::postEndSubRun(SubRun const&) {
   std::cout <<indention_<<indention_<<" finished end subRun:"<<std::endl;
}

void
Tracer::prePathEndSubRun(std::string const& iName) {
  std::cout <<indention_<<indention_<<indention_<<" processing path for end subRun:"<<iName<<std::endl;
}

void
Tracer::postPathEndSubRun(std::string const& iName, HLTPathStatus const&) {
  std::cout <<indention_<<indention_<<indention_<<" finished path for end subRun:"<<std::endl;
}

void
Tracer::preModuleEndSubRun(ModuleDescription const& iDescription) {
   ++depth_;
   std::cout <<indention_<<indention_<<indention_;
   for(unsigned int depth = 0; depth !=depth_; ++depth) {
      std::cout<<indention_;
   }
   std::cout<<" module for end subRun:" <<iDescription.moduleLabel_<<std::endl;
}

void
Tracer::postModuleEndSubRun(ModuleDescription const& iDescription) {
   --depth_;
   std::cout <<indention_<<indention_<<indention_<<indention_;
   for(unsigned int depth = 0; depth !=depth_; ++depth) {
      std::cout<<indention_;
   }

   std::cout<<" finished for end subRun:"<<iDescription.moduleLabel_<<std::endl;
}

void
Tracer::preSourceConstruction(ModuleDescription const& iDescription) {
  std::cout <<indention_;
  std::cout<<" constructing source:" <<iDescription.moduleName_<<std::endl;
}

void
Tracer::postSourceConstruction(ModuleDescription const& iDescription) {
  std::cout <<indention_;
  std::cout<<" construction finished:"<<iDescription.moduleName_<<std::endl;
}

void
Tracer::preModuleConstruction(ModuleDescription const& iDescription) {
  std::cout <<indention_;
  std::cout<<" constructing module:" <<iDescription.moduleLabel_<<std::endl;
}

void
Tracer::postModuleConstruction(ModuleDescription const& iDescription) {
  std::cout <<indention_;
  std::cout<<" construction finished:"<<iDescription.moduleLabel_<<std::endl;
}

void
Tracer::preModuleBeginJob(ModuleDescription const& iDescription) {
  std::cout <<indention_;
  std::cout<<" beginJob module:" <<iDescription.moduleLabel_<<std::endl;
}

void
Tracer::postModuleBeginJob(ModuleDescription const& iDescription) {
  std::cout <<indention_;
  std::cout<<" beginJob finished:"<<iDescription.moduleLabel_<<std::endl;
}

void
Tracer::preModuleEndJob(ModuleDescription const& iDescription) {
  std::cout <<indention_;
  std::cout<<" endJob module:" <<iDescription.moduleLabel_<<std::endl;
}

void
Tracer::postModuleEndJob(ModuleDescription const& iDescription) {
  std::cout <<indention_;
  std::cout<<" endJob finished:"<<iDescription.moduleLabel_<<std::endl;
}
