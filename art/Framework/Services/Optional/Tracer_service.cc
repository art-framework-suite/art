// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "art/Persistency/Provenance/ModuleContext.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/PathContext.h"
#include "art/Persistency/Provenance/ScheduleContext.h"
#include "art/Utilities/OutputFileInfo.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Persistency/Provenance/Timestamp.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Name.h"

#include <cassert>
#include <iostream>
#include <string>

using namespace std;
using namespace std::string_literals;

namespace art {

  class Tracer {
  public:
    struct Config {
      fhicl::Atom<string> indentation{fhicl::Name{"indentation"}, "++"};
    };
    using Parameters = ServiceTable<Config>;
    Tracer(Parameters const&, ActivityRegistry&);

  private:
    ostream& indent(unsigned n = 1u) const;

    void postBeginJob();
    void postEndJob();

    void preBeginRun(Run const& run);
    void postBeginRun(Run const& run);

    void preBeginSubRun(SubRun const& subRun);
    void postBeginSubRun(SubRun const& subRun);

    void preEvent(Event const& ev, ScheduleContext);
    void postEvent(Event const& ev, ScheduleContext);

    void preEndSubRun(SubRunID const& id, Timestamp const& ts);
    void postEndSubRun(SubRun const& run);

    void preEndRun(RunID const& id, Timestamp const& ts);
    void postEndRun(Run const& run);

    void preModuleConstruction(ModuleDescription const& md);
    void postModuleConstruction(ModuleDescription const& md);

    void preModuleBeginJob(ModuleDescription const& md);
    void postModuleBeginJob(ModuleDescription const& md);

    void preModuleBeginRun(ModuleContext const& mc);
    void postModuleBeginRun(ModuleContext const& mc);

    void preModuleBeginSubRun(ModuleContext const& mc);
    void postModuleBeginSubRun(ModuleContext const& mc);

    void preModuleEvent(ModuleContext const& mc);
    void postModuleEvent(ModuleContext const& mc);

    void preModuleEndSubRun(ModuleContext const& mc);
    void postModuleEndSubRun(ModuleContext const& mc);

    void preModuleEndRun(ModuleContext const& mc);
    void postModuleEndRun(ModuleContext const& mc);

    void preModuleEndJob(ModuleDescription const& md);
    void postModuleEndJob(ModuleDescription const& md);

    void preSourceEvent(ScheduleContext);
    void postSourceEvent(Event const&, ScheduleContext);

    void preSourceSubRun();
    void postSourceSubRun(SubRun const&);

    void preSourceRun();
    void postSourceRun(Run const&);

    void preOpenFile();
    void postOpenFile(string const& fn);

    void preCloseFile();
    void postCloseFile();

    void postOpenOutputFile(string const& label);
    void preCloseOutputFile(string const& label);
    void postCloseOutputFile(OutputFileInfo const& info);

    void prePathBeginRun(string const& s);
    void postPathBeginRun(string const& s, HLTPathStatus const& hlt);

    void prePathBeginSubRun(string const& s);
    void postPathBeginSubRun(string const& s, HLTPathStatus const& hlt);

    void prePathEvent(PathContext const& pc);
    void postPathEvent(PathContext const& pc, HLTPathStatus const& hlt);

    void prePathEndSubRun(string const& s);
    void postPathEndSubRun(string const& s, HLTPathStatus const& hlt);

    void prePathEndRun(string const& s);
    void postPathEndRun(string const& s, HLTPathStatus const& hlt);

    // Member Data
  private:
    string indentation_;
    unsigned int depth_{};
  };

  art::Tracer::Tracer(Parameters const& config, ActivityRegistry& iRegistry)
    : indentation_{config().indentation()}
  {
    iRegistry.sPostBeginJob.watch(this, &Tracer::postBeginJob);
    iRegistry.sPostEndJob.watch(this, &Tracer::postEndJob);
    iRegistry.sPreModule.watch(this, &Tracer::preModuleEvent);
    iRegistry.sPostModule.watch(this, &Tracer::postModuleEvent);
    iRegistry.sPreModuleConstruction.watch(this,
                                           &Tracer::preModuleConstruction);
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
    iRegistry.sPostModuleBeginSubRun.watch(this,
                                           &Tracer::postModuleBeginSubRun);
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

  ostream&
  Tracer::indent(unsigned n /*= 1u*/) const
  {
    for (; n != 0u; --n) {
      cout << indentation_;
    }
    return cout;
  }

  void
  art::Tracer::postBeginJob()
  {
    indent(1) << " Job started" << endl;
  }

  void
  art::Tracer::postEndJob()
  {
    indent(1) << " Job ended" << endl;
  }

  void art::Tracer::preSourceEvent(ScheduleContext)
  {
    indent(2) << "source event" << endl;
  }

  void
  art::Tracer::postSourceEvent(Event const&, ScheduleContext)
  {
    indent(2) << "finished: source event" << endl;
  }

  void
  art::Tracer::preSourceSubRun()
  {
    indent(2) << "source subRun" << endl;
  }

  void
  art::Tracer::postSourceSubRun(SubRun const&)
  {
    indent(2) << "finished: source subRun" << endl;
  }

  void
  art::Tracer::preSourceRun()
  {
    indent(2) << "source run" << endl;
  }

  void
  art::Tracer::postSourceRun(Run const&)
  {
    indent(2) << "finished: source run" << endl;
  }

  void
  art::Tracer::preOpenFile()
  {
    indent(2) << "open input file" << endl;
  }

  void
  Tracer::postOpenFile(string const& fn)
  {
    indent(2) << "finished: open input file" << fn << endl;
  }

  void
  art::Tracer::preCloseFile()
  {
    indent(2) << "close input file" << endl;
  }

  void
  art::Tracer::postCloseFile()
  {
    indent(2) << "finished: close input file" << endl;
  }

  void
  Tracer::postOpenOutputFile(string const& label)
  {
    indent(2) << "opened output file from " << label << endl;
  }

  void
  Tracer::preCloseOutputFile(string const& label)
  {
    indent(2) << "close output file from " << label << endl;
  }

  void
  art::Tracer::postCloseOutputFile(OutputFileInfo const& info)
  {
    string const fn{info.fileName().empty() ? "<none>"s : info.fileName()};
    indent(2) << "finished close output file " << fn << " from "
              << info.moduleLabel() << endl;
  }

  void
  art::Tracer::preEvent(Event const& ev, ScheduleContext)
  {
    depth_ = 0;
    indent(2) << " processing event:" << ev.id()
              << " time:" << ev.time().value() << endl;
  }

  void
  art::Tracer::postEvent(Event const&, ScheduleContext)
  {
    indent(2) << " finished event:" << endl;
  }

  void
  Tracer::prePathEvent(PathContext const& pc)
  {
    indent(3) << " processing path for event:" << pc.pathName() << endl;
  }

  void
  Tracer::postPathEvent(PathContext const& pc, HLTPathStatus const&)
  {
    indent(3) << " finished path for event:" << pc.pathName() << endl;
  }

  void
  Tracer::preModuleEvent(ModuleContext const& mc)
  {
    ++depth_;
    indent(3 + depth_) << " module for event:" << mc.moduleLabel() << endl;
  }

  void
  Tracer::postModuleEvent(ModuleContext const& mc)
  {
    --depth_;
    indent(4 + depth_) << " finished for event:" << mc.moduleLabel() << endl;
  }

  void
  art::Tracer::preBeginRun(Run const& run)
  {
    depth_ = 0;
    indent(2) << " processing begin run:" << run.id()
              << " time:" << run.beginTime().value() << endl;
  }

  void
  art::Tracer::postBeginRun(Run const&)
  {
    indent(2) << " finished begin run:" << endl;
  }

  void
  Tracer::prePathBeginRun(string const& iName)
  {
    indent(3) << " processing path for begin run:" << iName << endl;
  }

  void
  Tracer::postPathBeginRun(string const& /*iName*/, HLTPathStatus const&)
  {
    indent(3) << " finished path for begin run:" << endl;
  }

  void
  Tracer::preModuleBeginRun(ModuleContext const& mc)
  {
    ++depth_;
    indent(3 + depth_) << " module for begin run:" << mc.moduleLabel() << endl;
  }

  void
  Tracer::postModuleBeginRun(ModuleContext const& mc)
  {
    --depth_;
    indent(4 + depth_) << " finished for begin run:" << mc.moduleLabel()
                       << endl;
  }

  void
  art::Tracer::preEndRun(RunID const& iID, Timestamp const& iTime)
  {
    depth_ = 0;
    indent(2) << " processing end run:" << iID << " time:" << iTime.value()
              << endl;
  }

  void
  art::Tracer::postEndRun(Run const&)
  {
    indent(2) << " finished end run:" << endl;
  }

  void
  Tracer::prePathEndRun(string const& iName)
  {
    indent(3) << " processing path for end run:" << iName << endl;
  }

  void
  Tracer::postPathEndRun(string const& /*iName*/, HLTPathStatus const&)
  {
    indent(3) << " finished path for end run:" << endl;
  }

  void
  Tracer::preModuleEndRun(ModuleContext const& mc)
  {
    ++depth_;
    indent(3 + depth_) << " module for end run:" << mc.moduleLabel() << endl;
  }

  void
  Tracer::postModuleEndRun(ModuleContext const& mc)
  {
    --depth_;
    indent(4 + depth_) << " finished for end run:" << mc.moduleLabel() << endl;
  }

  void
  art::Tracer::preBeginSubRun(SubRun const& subRun)
  {
    depth_ = 0;
    indent(2) << " processing begin subRun:" << subRun.id()
              << " time:" << subRun.beginTime().value() << endl;
  }

  void
  art::Tracer::postBeginSubRun(SubRun const&)
  {
    indent(2) << " finished begin subRun:" << endl;
  }

  void
  Tracer::prePathBeginSubRun(string const& iName)
  {
    indent(3) << " processing path for begin subRun:" << iName << endl;
  }

  void
  Tracer::postPathBeginSubRun(string const& /*iName*/, HLTPathStatus const&)
  {
    indent(3) << " finished path for begin subRun:" << endl;
  }

  void
  Tracer::preModuleBeginSubRun(ModuleContext const& mc)
  {
    ++depth_;
    indent(3 + depth_) << " module for begin subRun:" << mc.moduleLabel()
                       << endl;
  }

  void
  Tracer::postModuleBeginSubRun(ModuleContext const& mc)
  {
    --depth_;
    indent(4) << " finished for begin subRun:" << mc.moduleLabel() << endl;
  }

  void
  art::Tracer::preEndSubRun(SubRunID const& iID, Timestamp const& iTime)
  {
    depth_ = 0;
    indent(2) << " processing end subRun:" << iID << " time:" << iTime.value()
              << endl;
  }

  void
  art::Tracer::postEndSubRun(SubRun const&)
  {
    indent(2) << " finished end subRun:" << endl;
  }

  void
  Tracer::prePathEndSubRun(string const& iName)
  {
    indent(3) << " processing path for end subRun:" << iName << endl;
  }

  void
  Tracer::postPathEndSubRun(string const& /*iName*/, HLTPathStatus const&)
  {
    indent(3) << " finished path for end subRun:" << endl;
  }

  void
  Tracer::preModuleEndSubRun(ModuleContext const& mc)
  {
    ++depth_;
    indent(3 + depth_) << " module for end subRun:" << mc.moduleLabel() << endl;
  }

  void
  Tracer::postModuleEndSubRun(ModuleContext const& mc)
  {
    --depth_;
    indent(4 + depth_) << " finished for end subRun:" << mc.moduleLabel()
                       << endl;
  }

  void
  Tracer::preModuleConstruction(ModuleDescription const& md)
  {
    indent(1) << " constructing module:" << md.moduleLabel() << endl;
  }

  void
  Tracer::postModuleConstruction(ModuleDescription const& md)
  {
    indent(1) << " construction finished:" << md.moduleLabel() << endl;
  }

  void
  Tracer::preModuleBeginJob(ModuleDescription const& md)
  {
    indent(1) << " beginJob module:" << md.moduleLabel() << endl;
  }

  void
  Tracer::postModuleBeginJob(ModuleDescription const& md)
  {
    indent(1) << " beginJob finished:" << md.moduleLabel() << endl;
  }

  void
  Tracer::preModuleEndJob(ModuleDescription const& md)
  {
    indent(1) << " endJob module:" << md.moduleLabel() << endl;
  }

  void
  Tracer::postModuleEndJob(ModuleDescription const& md)
  {
    indent(1) << " endJob finished:" << md.moduleLabel() << endl;
  }

} // namespace art

DECLARE_ART_SERVICE(art::Tracer, LEGACY)
DEFINE_ART_SERVICE(art::Tracer)
