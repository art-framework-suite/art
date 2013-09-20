// ======================================================================
//
// Package:     Services
// Class  :     Tracer
//
// ======================================================================

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "art/Persistency/Provenance/Timestamp.h"
#include "fhiclcpp/ParameterSet.h"
#include <iostream>

namespace art {
  class Tracer;
}

using art::Tracer;

// ======================================================================

class Tracer
{
public:
  Tracer(fhicl::ParameterSet const&, ActivityRegistry&);

  void postBeginJob();
  void postEndJob();

  void preBeginRun(Run const& run);
  void postBeginRun(Run const& run);

  void preBeginSubRun(SubRun const& subRun);
  void postBeginSubRun(SubRun const& subRun);

  void preEvent(Event const& ev);
  void postEvent(Event const& ev);

  void preEndSubRun(SubRunID const& id, Timestamp const& ts);
  void postEndSubRun(SubRun const& run);

  void preEndRun(RunID const& id, Timestamp const& ts);
  void postEndRun(Run const& run);

  void preModuleConstruction(ModuleDescription const& md);
  void postModuleConstruction(ModuleDescription const& md);

  void preModuleBeginJob(ModuleDescription const& md);
  void postModuleBeginJob(ModuleDescription const& md);

  void preModuleBeginRun(ModuleDescription const& md);
  void postModuleBeginRun(ModuleDescription const& md);

  void preModuleBeginSubRun(ModuleDescription const& md);
  void postModuleBeginSubRun(ModuleDescription const& md);

  void preModuleEvent(ModuleDescription const& md);
  void postModuleEvent(ModuleDescription const& md);

  void preModuleEndSubRun(ModuleDescription const& md);
  void postModuleEndSubRun(ModuleDescription const& md);

  void preModuleEndRun(ModuleDescription const& md);
  void postModuleEndRun(ModuleDescription const& md);

  void preModuleEndJob(ModuleDescription const& md);
  void postModuleEndJob(ModuleDescription const& md);

  void preSourceEvent();
  void postSourceEvent();

  void preSourceSubRun();
  void postSourceSubRun();

  void preSourceRun();
  void postSourceRun();

  void preOpenFile();
  void postOpenFile(std::string const &fn);

  void preCloseFile();
  void postCloseFile();

  void prePathBeginRun(std::string const& s);
  void postPathBeginRun(std::string const& s, HLTPathStatus const& hlt);

  void prePathBeginSubRun(std::string const& s);
  void postPathBeginSubRun(std::string const& s, HLTPathStatus const& hlt);

  void prePathEvent(std::string const& s);
  void postPathEvent(std::string const& s, HLTPathStatus const& hlt);

  void prePathEndSubRun(std::string const& s);
  void postPathEndSubRun(std::string const& s, HLTPathStatus const& hlt);

  void prePathEndRun(std::string const& s);
  void postPathEndRun(std::string const& s, HLTPathStatus const& hlt);

private:
  std::string indentation_;
  unsigned int depth_;

  std::ostream & indent( unsigned n = 1u ) const
  {
    for( ; n != 0u; --n )
      std::cout << indentation_;
    return std::cout;
  }

};  // Tracer

// ======================================================================
// constructors and destructor

Tracer::Tracer(fhicl::ParameterSet const& iPS, ActivityRegistry&iRegistry)
: indentation_( iPS.get<std::string>("indentation", "++") )
, depth_      ( 0 )
{
  iRegistry.sPostBeginJob.watch(&Tracer::postBeginJob, *this);
  iRegistry.sPostEndJob.watch(&Tracer::postEndJob, *this);

  iRegistry.sPreModule.watchAll(&Tracer::preModuleEvent, *this);
  iRegistry.sPostModule.watchAll(&Tracer::postModuleEvent, *this);

  iRegistry.sPreModuleConstruction.watch(&Tracer::preModuleConstruction, *this);
  iRegistry.sPostModuleConstruction.watch(&Tracer::postModuleConstruction, *this);

  iRegistry.sPreModuleBeginJob.watch(&Tracer::preModuleBeginJob, *this);
  iRegistry.sPostModuleBeginJob.watch(&Tracer::postModuleBeginJob, *this);

  iRegistry.sPreModuleEndJob.watch(&Tracer::preModuleEndJob, *this);
  iRegistry.sPostModuleEndJob.watch(&Tracer::postModuleEndJob, *this);

  iRegistry.sPreModuleBeginRun.watch(&Tracer::preModuleBeginRun, *this);
  iRegistry.sPostModuleBeginRun.watch(&Tracer::postModuleBeginRun, *this);

  iRegistry.sPreModuleEndRun.watch(&Tracer::preModuleEndRun, *this);
  iRegistry.sPostModuleEndRun.watch(&Tracer::postModuleEndRun, *this);

  iRegistry.sPreModuleBeginSubRun.watch(&Tracer::preModuleBeginSubRun, *this);
  iRegistry.sPostModuleBeginSubRun.watch(&Tracer::postModuleBeginSubRun, *this);

  iRegistry.sPreModuleEndSubRun.watch(&Tracer::preModuleEndSubRun, *this);
  iRegistry.sPostModuleEndSubRun.watch(&Tracer::postModuleEndSubRun, *this);

  iRegistry.sPreProcessPath.watchAll(&Tracer::prePathEvent, *this);
  iRegistry.sPostProcessPath.watchAll(&Tracer::postPathEvent, *this);

  iRegistry.sPrePathBeginRun.watch(&Tracer::prePathBeginRun, *this);
  iRegistry.sPostPathBeginRun.watch(&Tracer::postPathBeginRun, *this);

  iRegistry.sPrePathEndRun.watch(&Tracer::prePathEndRun, *this);
  iRegistry.sPostPathEndRun.watch(&Tracer::postPathEndRun, *this);

  iRegistry.sPrePathBeginSubRun.watch(&Tracer::prePathBeginSubRun, *this);
  iRegistry.sPostPathBeginSubRun.watch(&Tracer::postPathBeginSubRun, *this);

  iRegistry.sPrePathEndSubRun.watch(&Tracer::prePathEndSubRun, *this);
  iRegistry.sPostPathEndSubRun.watch(&Tracer::postPathEndSubRun, *this);

  iRegistry.sPreProcessEvent.watchAll(&Tracer::preEvent, *this);
  iRegistry.sPostProcessEvent.watchAll(&Tracer::postEvent, *this);

  iRegistry.sPreBeginRun.watch(&Tracer::preBeginRun, *this);
  iRegistry.sPostBeginRun.watch(&Tracer::postBeginRun, *this);

  iRegistry.sPreEndRun.watch(&Tracer::preEndRun, *this);
  iRegistry.sPostEndRun.watch(&Tracer::postEndRun, *this);

  iRegistry.sPreBeginSubRun.watch(&Tracer::preBeginSubRun, *this);
  iRegistry.sPostBeginSubRun.watch(&Tracer::postBeginSubRun, *this);

  iRegistry.sPreEndSubRun.watch(&Tracer::preEndSubRun, *this);
  iRegistry.sPostEndSubRun.watch(&Tracer::postEndSubRun, *this);

  iRegistry.sPreSource.watch(&Tracer::preSourceEvent, *this);
  iRegistry.sPostSource.watch(&Tracer::postSourceEvent, *this);

  iRegistry.sPreOpenFile.watch(&Tracer::preOpenFile, *this);
  iRegistry.sPostOpenFile.watch(&Tracer::postOpenFile, *this);

  iRegistry.sPreCloseFile.watch(&Tracer::preCloseFile, *this);
  iRegistry.sPostCloseFile.watch(&Tracer::postCloseFile, *this);

  iRegistry.sPreSourceRun.watch(&Tracer::preSourceRun, *this);
  iRegistry.sPostSourceRun.watch(&Tracer::postSourceRun, *this);

  iRegistry.sPreSourceSubRun.watch(&Tracer::preSourceSubRun, *this);
  iRegistry.sPostSourceSubRun.watch(&Tracer::postSourceSubRun, *this);
}

// ======================================================================
// member functions

void
Tracer::postBeginJob() {
   indent(1) << " Job started" << std::endl;
}
void
Tracer::postEndJob() {
   indent(1) << " Job ended" << std::endl;
}

void
Tracer::preSourceEvent() {
  indent(2) << "source event" << std::endl;
}
void
Tracer::postSourceEvent () {
  indent(2) << "finished: source event" << std::endl;
}

void
Tracer::preSourceSubRun() {
  indent(2) << "source subRun" << std::endl;
}
void
Tracer::postSourceSubRun () {
  indent(2) << "finished: source subRun" << std::endl;
}

void
Tracer::preSourceRun() {
  indent(2) << "source run" << std::endl;
}
void
Tracer::postSourceRun () {
  indent(2) << "finished: source run" << std::endl;
}

void
Tracer::preOpenFile() {
  indent(2) << "open input file" << std::endl;
}
void
Tracer::postOpenFile (std::string const &fn) {
  indent(2) << "finished: open input file "
            << fn
            << std::endl;
}

void
Tracer::preCloseFile() {
  indent(2) << "close input file" << std::endl;
}
void
Tracer::postCloseFile () {
  indent(2) << "finished: close input file" << std::endl;
}

void
Tracer::preEvent(Event const& ev) {
   depth_=0;
   indent(2) << " processing event:"
             << ev.id()
             << " time:"
             << ev.time().value()
             << std::endl;
}
void
Tracer::postEvent(Event const&) {
   indent(2) << " finished event:" << std::endl;
}

void
Tracer::prePathEvent(std::string const& iName) {
  indent(3) << " processing path for event:" << iName << std::endl;
}
void
Tracer::postPathEvent(std::string const& /*iName*/, HLTPathStatus const&) {
  indent(3) << " finished path for event:" << std::endl;
}

void
Tracer::preModuleEvent(ModuleDescription const& iDescription) {
   ++depth_;
   indent(3+depth_) << " module for event:" << iDescription.moduleLabel() << std::endl;
}
void
Tracer::postModuleEvent(ModuleDescription const& iDescription) {
   --depth_;
   indent(4+depth_) << " finished for event:" << iDescription.moduleLabel() << std::endl;
}

void
Tracer::preBeginRun(Run const& run) {
   depth_=0;
   indent(2) << " processing begin run:"
             << run.id()
             << " time:"
             << run.beginTime().value()
             << std::endl;
}
void
Tracer::postBeginRun(Run const&) {
   indent(2) << " finished begin run:" << std::endl;
}

void
Tracer::prePathBeginRun(std::string const& iName) {
  indent(3) << " processing path for begin run:" << iName << std::endl;
}

void
Tracer::postPathBeginRun(std::string const& /*iName*/, HLTPathStatus const&) {
  indent(3) << " finished path for begin run:" << std::endl;
}

void
Tracer::preModuleBeginRun(ModuleDescription const& iDescription) {
   ++depth_;
   indent(3+depth_) << " module for begin run:" << iDescription.moduleLabel() << std::endl;
}
void
Tracer::postModuleBeginRun(ModuleDescription const& iDescription) {
   --depth_;
   indent(4+depth_) << " finished for begin run:" << iDescription.moduleLabel() << std::endl;
}

void
Tracer::preEndRun(RunID const& iID, Timestamp const& iTime) {
   depth_=0;
   indent(2) << " processing end run:" << iID << " time:" << iTime.value() << std::endl;
}

void
Tracer::postEndRun(Run const&) {
   indent(2) << " finished end run:" << std::endl;
}

void
Tracer::prePathEndRun(std::string const& iName) {
  indent(3) << " processing path for end run:" << iName << std::endl;
}
void
Tracer::postPathEndRun(std::string const& /*iName*/, HLTPathStatus const&) {
  indent(3) << " finished path for end run:" << std::endl;
}

void
Tracer::preModuleEndRun(ModuleDescription const& iDescription) {
   ++depth_;
   indent(3+depth_) << " module for end run:" << iDescription.moduleLabel() << std::endl;
}
void
Tracer::postModuleEndRun(ModuleDescription const& iDescription) {
   --depth_;
   indent(4+depth_) << " finished for end run:" << iDescription.moduleLabel() << std::endl;
}

void
Tracer::preBeginSubRun(SubRun const& subRun) {
   depth_=0;
   indent(2) << " processing begin subRun:"
             << subRun.id()
             << " time:"
             << subRun.beginTime().value()
             << std::endl;
}

void
Tracer::postBeginSubRun(SubRun const&) {
   indent(2) << " finished begin subRun:" << std::endl;
}

void
Tracer::prePathBeginSubRun(std::string const& iName) {
  indent(3) << " processing path for begin subRun:" << iName << std::endl;
}
void
Tracer::postPathBeginSubRun(std::string const& /*iName*/, HLTPathStatus const&) {
  indent(3) << " finished path for begin subRun:" << std::endl;
}

void
Tracer::preModuleBeginSubRun(ModuleDescription const& iDescription) {
   ++depth_;
   indent(3+depth_) << " module for begin subRun:" << iDescription.moduleLabel() << std::endl;
}
void
Tracer::postModuleBeginSubRun(ModuleDescription const& iDescription) {
   --depth_;
   indent(4) << " finished for begin subRun:" << iDescription.moduleLabel() << std::endl;
}

void
Tracer::preEndSubRun(SubRunID const& iID, Timestamp const& iTime) {
   depth_=0;
   indent(2) << " processing end subRun:" << iID << " time:" << iTime.value() << std::endl;
}
void
Tracer::postEndSubRun(SubRun const&) {
   indent(2) << " finished end subRun:" << std::endl;
}

void
Tracer::prePathEndSubRun(std::string const& iName) {
  indent(3) << " processing path for end subRun:" << iName << std::endl;
}

void
Tracer::postPathEndSubRun(std::string const& /*iName*/, HLTPathStatus const&) {
  indent(3) << " finished path for end subRun:" << std::endl;
}

void
Tracer::preModuleEndSubRun(ModuleDescription const& iDescription) {
   ++depth_;
   indent(3+depth_) << " module for end subRun:" << iDescription.moduleLabel() << std::endl;
}

void
Tracer::postModuleEndSubRun(ModuleDescription const& iDescription) {
   --depth_;
   indent(4+depth_) << " finished for end subRun:" << iDescription.moduleLabel() << std::endl;
}

void
Tracer::preModuleConstruction(ModuleDescription const& iDescription) {
  indent(1) << " constructing module:" << iDescription.moduleLabel() << std::endl;
}

void
Tracer::postModuleConstruction(ModuleDescription const& iDescription) {
  indent(1) << " construction finished:" << iDescription.moduleLabel() << std::endl;
}

void
Tracer::preModuleBeginJob(ModuleDescription const& iDescription) {
  indent(1) << " beginJob module:" << iDescription.moduleLabel() << std::endl;
}

void
Tracer::postModuleBeginJob(ModuleDescription const& iDescription) {
  indent(1) << " beginJob finished:" << iDescription.moduleLabel() << std::endl;
}

void
Tracer::preModuleEndJob(ModuleDescription const& iDescription) {
  indent(1) << " endJob module:" << iDescription.moduleLabel() << std::endl;
}

void
Tracer::postModuleEndJob(ModuleDescription const& iDescription) {
  indent(1) << " endJob finished:" << iDescription.moduleLabel() << std::endl;
}

// ======================================================================

// The DECLARE macro call should be moved to the header file, should you
// create one.
DECLARE_ART_SERVICE(Tracer, LEGACY)
DEFINE_ART_SERVICE(Tracer)

// ======================================================================
