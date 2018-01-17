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
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "art/Utilities/OutputFileInfo.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Persistency/Provenance/Timestamp.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Name.h"

#include <cassert>
#include <iostream>
#include <string>

using namespace std::string_literals;

namespace art {
  class Tracer;
}

// ======================================================================
class art::Tracer {
public:
  struct Config {
    fhicl::Atom<std::string> indentation{fhicl::Name{"indentation"}, "++"};
  };

  using Parameters = ServiceTable<Config>;
  Tracer(ServiceTable<Config> const&, ActivityRegistry&);

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
  void postSourceEvent(Event const&);

  void preSourceSubRun();
  void postSourceSubRun(SubRun const&);

  void preSourceRun();
  void postSourceRun(Run const&);

  void preOpenFile();
  void postOpenFile(std::string const& fn);

  void preCloseFile();
  void postCloseFile();

  void postOpenOutputFile(std::string const& label);
  void preCloseOutputFile(std::string const& label);
  void postCloseOutputFile(OutputFileInfo const& info);

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
  unsigned int depth_{};

  std::ostream&
  indent(unsigned n = 1u) const
  {
    for (; n != 0u; --n)
      std::cout << indentation_;
    return std::cout;
  }

}; // Tracer

// ======================================================================
// constructors and destructor

art::Tracer::Tracer(ServiceTable<Config> const& config,
                    ActivityRegistry& iRegistry)
  : indentation_{config().indentation()}
{
  iRegistry.sPostBeginJob.watch(this, &Tracer::postBeginJob);
  iRegistry.sPostEndJob.watch(this, &Tracer::postEndJob);

  iRegistry.sPreModule.watch(this, &Tracer::preModuleEvent);
  iRegistry.sPostModule.watch(this, &Tracer::postModuleEvent);

  iRegistry.sPreModuleConstruction.watch(this, &Tracer::preModuleConstruction);
  iRegistry.sPostModuleConstruction.watch(this,
                                          &Tracer::postModuleConstruction);

  iRegistry.sPreModuleBeginJob.watch(this, &Tracer::preModuleBeginJob);
  iRegistry.sPostModuleBeginJob.watch(this, &Tracer::postModuleBeginJob);

  iRegistry.sPreModuleEndJob.watch(this, &Tracer::preModuleEndJob);
  iRegistry.sPostModuleEndJob.watch(this, &Tracer::postModuleEndJob);

  iRegistry.sPreModuleBeginRun.watch(this, &Tracer::preModuleBeginRun);
  iRegistry.sPostModuleBeginRun.watch(this, &Tracer::postModuleBeginRun);

  iRegistry.sPreModuleEndRun.watch(this, &Tracer::preModuleEndRun);
  iRegistry.sPostModuleEndRun.watch(this, &Tracer::postModuleEndRun);

  iRegistry.sPreModuleBeginSubRun.watch(this, &Tracer::preModuleBeginSubRun);
  iRegistry.sPostModuleBeginSubRun.watch(this, &Tracer::postModuleBeginSubRun);

  iRegistry.sPreModuleEndSubRun.watch(this, &Tracer::preModuleEndSubRun);
  iRegistry.sPostModuleEndSubRun.watch(this, &Tracer::postModuleEndSubRun);

  iRegistry.sPreProcessPath.watch(this, &Tracer::prePathEvent);
  iRegistry.sPostProcessPath.watch(this, &Tracer::postPathEvent);

  iRegistry.sPrePathBeginRun.watch(this, &Tracer::prePathBeginRun);
  iRegistry.sPostPathBeginRun.watch(this, &Tracer::postPathBeginRun);

  iRegistry.sPrePathEndRun.watch(this, &Tracer::prePathEndRun);
  iRegistry.sPostPathEndRun.watch(this, &Tracer::postPathEndRun);

  iRegistry.sPrePathBeginSubRun.watch(this, &Tracer::prePathBeginSubRun);
  iRegistry.sPostPathBeginSubRun.watch(this, &Tracer::postPathBeginSubRun);

  iRegistry.sPrePathEndSubRun.watch(this, &Tracer::prePathEndSubRun);
  iRegistry.sPostPathEndSubRun.watch(this, &Tracer::postPathEndSubRun);

  iRegistry.sPreProcessEvent.watch(this, &Tracer::preEvent);
  iRegistry.sPostProcessEvent.watch(this, &Tracer::postEvent);

  iRegistry.sPreBeginRun.watch(this, &Tracer::preBeginRun);
  iRegistry.sPostBeginRun.watch(this, &Tracer::postBeginRun);

  iRegistry.sPreEndRun.watch(this, &Tracer::preEndRun);
  iRegistry.sPostEndRun.watch(this, &Tracer::postEndRun);

  iRegistry.sPreBeginSubRun.watch(this, &Tracer::preBeginSubRun);
  iRegistry.sPostBeginSubRun.watch(this, &Tracer::postBeginSubRun);

  iRegistry.sPreEndSubRun.watch(this, &Tracer::preEndSubRun);
  iRegistry.sPostEndSubRun.watch(this, &Tracer::postEndSubRun);

  iRegistry.sPreSourceEvent.watch(this, &Tracer::preSourceEvent);
  iRegistry.sPostSourceEvent.watch(this, &Tracer::postSourceEvent);

  iRegistry.sPreOpenFile.watch(this, &Tracer::preOpenFile);
  iRegistry.sPostOpenFile.watch(this, &Tracer::postOpenFile);

  iRegistry.sPreCloseFile.watch(this, &Tracer::preCloseFile);
  iRegistry.sPostCloseFile.watch(this, &Tracer::postCloseFile);

  iRegistry.sPostOpenOutputFile.watch(this, &Tracer::postOpenOutputFile);
  iRegistry.sPreCloseOutputFile.watch(this, &Tracer::preCloseOutputFile);
  iRegistry.sPostCloseOutputFile.watch(this, &Tracer::postCloseOutputFile);

  iRegistry.sPreSourceRun.watch(this, &Tracer::preSourceRun);
  iRegistry.sPostSourceRun.watch(this, &Tracer::postSourceRun);

  iRegistry.sPreSourceSubRun.watch(this, &Tracer::preSourceSubRun);
  iRegistry.sPostSourceSubRun.watch(this, &Tracer::postSourceSubRun);
}

// ======================================================================
// member functions

void
art::Tracer::postBeginJob()
{
  indent(1) << " Job started" << std::endl;
}

void
art::Tracer::postEndJob()
{
  indent(1) << " Job ended" << std::endl;
}

void
art::Tracer::preSourceEvent()
{
  indent(2) << "source event" << std::endl;
}

void
art::Tracer::postSourceEvent(Event const&)
{
  indent(2) << "finished: source event" << std::endl;
}

void
art::Tracer::preSourceSubRun()
{
  indent(2) << "source subRun" << std::endl;
}

void
art::Tracer::postSourceSubRun(SubRun const&)
{
  indent(2) << "finished: source subRun" << std::endl;
}

void
art::Tracer::preSourceRun()
{
  indent(2) << "source run" << std::endl;
}

void
art::Tracer::postSourceRun(Run const&)
{
  indent(2) << "finished: source run" << std::endl;
}

void
art::Tracer::preOpenFile()
{
  indent(2) << "open input file" << std::endl;
}

void
art::Tracer::postOpenFile(std::string const& fn)
{
  indent(2) << "finished: open input file" << fn << std::endl;
}

void
art::Tracer::preCloseFile()
{
  indent(2) << "close input file" << std::endl;
}

void
art::Tracer::postCloseFile()
{
  indent(2) << "finished: close input file" << std::endl;
}

void
art::Tracer::postOpenOutputFile(std::string const& label)
{
  indent(2) << "opened output file from " << label << std::endl;
}

void
art::Tracer::preCloseOutputFile(std::string const& label)
{
  indent(2) << "close output file from " << label << std::endl;
}

void
art::Tracer::postCloseOutputFile(OutputFileInfo const& info)
{
  std::string const fn{info.fileName().empty() ? "<none>"s : info.fileName()};
  indent(2) << "finished close output file " << fn << " from "
            << info.moduleLabel() << std::endl;
}

void
art::Tracer::preEvent(Event const& ev)
{
  depth_ = 0;
  indent(2) << " processing event:" << ev.id() << " time:" << ev.time().value()
            << std::endl;
}

void
art::Tracer::postEvent(Event const&)
{
  indent(2) << " finished event:" << std::endl;
}

void
art::Tracer::prePathEvent(std::string const& iName)
{
  indent(3) << " processing path for event:" << iName << std::endl;
}

void
art::Tracer::postPathEvent(std::string const& /*iName*/, HLTPathStatus const&)
{
  indent(3) << " finished path for event:" << std::endl;
}

void
art::Tracer::preModuleEvent(ModuleDescription const& iDescription)
{
  ++depth_;
  indent(3 + depth_) << " module for event:" << iDescription.moduleLabel()
                     << std::endl;
}

void
art::Tracer::postModuleEvent(ModuleDescription const& iDescription)
{
  --depth_;
  indent(4 + depth_) << " finished for event:" << iDescription.moduleLabel()
                     << std::endl;
}

void
art::Tracer::preBeginRun(Run const& run)
{
  depth_ = 0;
  indent(2) << " processing begin run:" << run.id()
            << " time:" << run.beginTime().value() << std::endl;
}

void
art::Tracer::postBeginRun(Run const&)
{
  indent(2) << " finished begin run:" << std::endl;
}

void
art::Tracer::prePathBeginRun(std::string const& iName)
{
  indent(3) << " processing path for begin run:" << iName << std::endl;
}

void
art::Tracer::postPathBeginRun(std::string const& /*iName*/,
                              HLTPathStatus const&)
{
  indent(3) << " finished path for begin run:" << std::endl;
}

void
art::Tracer::preModuleBeginRun(ModuleDescription const& iDescription)
{
  ++depth_;
  indent(3 + depth_) << " module for begin run:" << iDescription.moduleLabel()
                     << std::endl;
}

void
art::Tracer::postModuleBeginRun(ModuleDescription const& iDescription)
{
  --depth_;
  indent(4 + depth_) << " finished for begin run:" << iDescription.moduleLabel()
                     << std::endl;
}

void
art::Tracer::preEndRun(RunID const& iID, Timestamp const& iTime)
{
  depth_ = 0;
  indent(2) << " processing end run:" << iID << " time:" << iTime.value()
            << std::endl;
}

void
art::Tracer::postEndRun(Run const&)
{
  indent(2) << " finished end run:" << std::endl;
}

void
art::Tracer::prePathEndRun(std::string const& iName)
{
  indent(3) << " processing path for end run:" << iName << std::endl;
}

void
art::Tracer::postPathEndRun(std::string const& /*iName*/, HLTPathStatus const&)
{
  indent(3) << " finished path for end run:" << std::endl;
}

void
art::Tracer::preModuleEndRun(ModuleDescription const& iDescription)
{
  ++depth_;
  indent(3 + depth_) << " module for end run:" << iDescription.moduleLabel()
                     << std::endl;
}

void
art::Tracer::postModuleEndRun(ModuleDescription const& iDescription)
{
  --depth_;
  indent(4 + depth_) << " finished for end run:" << iDescription.moduleLabel()
                     << std::endl;
}

void
art::Tracer::preBeginSubRun(SubRun const& subRun)
{
  depth_ = 0;
  indent(2) << " processing begin subRun:" << subRun.id()
            << " time:" << subRun.beginTime().value() << std::endl;
}

void
art::Tracer::postBeginSubRun(SubRun const&)
{
  indent(2) << " finished begin subRun:" << std::endl;
}

void
art::Tracer::prePathBeginSubRun(std::string const& iName)
{
  indent(3) << " processing path for begin subRun:" << iName << std::endl;
}

void
art::Tracer::postPathBeginSubRun(std::string const& /*iName*/,
                                 HLTPathStatus const&)
{
  indent(3) << " finished path for begin subRun:" << std::endl;
}

void
art::Tracer::preModuleBeginSubRun(ModuleDescription const& iDescription)
{
  ++depth_;
  indent(3 + depth_) << " module for begin subRun:"
                     << iDescription.moduleLabel() << std::endl;
}
void
art::Tracer::postModuleBeginSubRun(ModuleDescription const& iDescription)
{
  --depth_;
  indent(4) << " finished for begin subRun:" << iDescription.moduleLabel()
            << std::endl;
}

void
art::Tracer::preEndSubRun(SubRunID const& iID, Timestamp const& iTime)
{
  depth_ = 0;
  indent(2) << " processing end subRun:" << iID << " time:" << iTime.value()
            << std::endl;
}
void
art::Tracer::postEndSubRun(SubRun const&)
{
  indent(2) << " finished end subRun:" << std::endl;
}

void
art::Tracer::prePathEndSubRun(std::string const& iName)
{
  indent(3) << " processing path for end subRun:" << iName << std::endl;
}

void
art::Tracer::postPathEndSubRun(std::string const& /*iName*/,
                               HLTPathStatus const&)
{
  indent(3) << " finished path for end subRun:" << std::endl;
}

void
art::Tracer::preModuleEndSubRun(ModuleDescription const& iDescription)
{
  ++depth_;
  indent(3 + depth_) << " module for end subRun:" << iDescription.moduleLabel()
                     << std::endl;
}

void
art::Tracer::postModuleEndSubRun(ModuleDescription const& iDescription)
{
  --depth_;
  indent(4 + depth_) << " finished for end subRun:"
                     << iDescription.moduleLabel() << std::endl;
}

void
art::Tracer::preModuleConstruction(ModuleDescription const& iDescription)
{
  indent(1) << " constructing module:" << iDescription.moduleLabel()
            << std::endl;
}

void
art::Tracer::postModuleConstruction(ModuleDescription const& iDescription)
{
  indent(1) << " construction finished:" << iDescription.moduleLabel()
            << std::endl;
}

void
art::Tracer::preModuleBeginJob(ModuleDescription const& iDescription)
{
  indent(1) << " beginJob module:" << iDescription.moduleLabel() << std::endl;
}

void
art::Tracer::postModuleBeginJob(ModuleDescription const& iDescription)
{
  indent(1) << " beginJob finished:" << iDescription.moduleLabel() << std::endl;
}

void
art::Tracer::preModuleEndJob(ModuleDescription const& iDescription)
{
  indent(1) << " endJob module:" << iDescription.moduleLabel() << std::endl;
}

void
art::Tracer::postModuleEndJob(ModuleDescription const& iDescription)
{
  indent(1) << " endJob finished:" << iDescription.moduleLabel() << std::endl;
}

// ======================================================================
DECLARE_ART_SERVICE(art::Tracer, LEGACY)
DEFINE_ART_SERVICE(art::Tracer)
