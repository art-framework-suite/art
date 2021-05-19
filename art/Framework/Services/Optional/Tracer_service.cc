// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceDeclarationMacros.h"
#include "art/Framework/Services/Registry/ServiceDefinitionMacros.h"
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "art/Persistency/Provenance/ModuleContext.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/PathContext.h"
#include "art/Persistency/Provenance/ScheduleContext.h"
#include "art/Utilities/Globals.h"
#include "art/Utilities/OutputFileInfo.h"
#include "canvas/Persistency/Common/fwd.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Persistency/Provenance/Timestamp.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Name.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <atomic>
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
    void log_with_indent(unsigned n, std::string const& message) const;

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
    string const indentation_;
    std::atomic<unsigned int> depth_{};
  };

  art::Tracer::Tracer(Parameters const& config, ActivityRegistry& iRegistry)
    : indentation_{config().indentation()}
  {
    if (auto const nthreads = Globals::instance()->nthreads(); nthreads != 1) {
      mf::LogWarning("Tracer") << "Because " << nthreads
                               << " threads have been configured, the tracing "
                                  "messages will be interleaved.\n"
                               << "Please configure your job to use one thread "
                                  "for a predictable output.";
    }

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

  void
  Tracer::log_with_indent(unsigned n, std::string const& message) const
  {
    std::string printout;
    for (; n != 0u; --n) {
      printout.append(indentation_);
    }
    printout += ' ';
    printout += message;
    printout += '\n';
    std::cout << printout;
  }

  void
  art::Tracer::postBeginJob()
  {
    log_with_indent(1, "Job started");
  }

  void
  art::Tracer::postEndJob()
  {
    log_with_indent(1, "Job ended");
  }

  void art::Tracer::preSourceEvent(ScheduleContext)
  {
    log_with_indent(2, "source event");
  }

  void
  art::Tracer::postSourceEvent(Event const&, ScheduleContext)
  {
    log_with_indent(2, "finished source event");
  }

  void
  art::Tracer::preSourceSubRun()
  {
    log_with_indent(2, "source subRun");
  }

  void
  art::Tracer::postSourceSubRun(SubRun const&)
  {
    log_with_indent(2, "finished source subRun");
  }

  void
  art::Tracer::preSourceRun()
  {
    log_with_indent(2, "source run");
  }

  void
  art::Tracer::postSourceRun(Run const&)
  {
    log_with_indent(2, "finished source run");
  }

  void
  art::Tracer::preOpenFile()
  {
    log_with_indent(2, "open input file");
  }

  void
  Tracer::postOpenFile(string const& fn)
  {
    string const displayed_fn{fn.empty() ? "<none>"s : fn};
    log_with_indent(2, "finished open input file: " + displayed_fn);
  }

  void
  art::Tracer::preCloseFile()
  {
    log_with_indent(2, "close input file");
  }

  void
  art::Tracer::postCloseFile()
  {
    log_with_indent(2, "finished close input file");
  }

  void
  Tracer::postOpenOutputFile(string const& label)
  {
    log_with_indent(2, "opened output file from " + label);
  }

  void
  Tracer::preCloseOutputFile(string const& label)
  {
    log_with_indent(2, "close output file from " + label);
  }

  void
  art::Tracer::postCloseOutputFile(OutputFileInfo const& info)
  {
    string const fn{info.fileName().empty() ? "<none>"s : info.fileName()};
    log_with_indent(
      2, "finished close output file " + fn + " from " + info.moduleLabel());
  }

  void
  art::Tracer::preEvent(Event const& ev, ScheduleContext)
  {
    depth_ = 0;
    std::ostringstream msg;
    msg << "processing event: " << ev.id() << " time: " << ev.time().value();
    log_with_indent(2, msg.str());
  }

  void
  art::Tracer::postEvent(Event const&, ScheduleContext)
  {
    log_with_indent(2, "finished event");
  }

  void
  Tracer::prePathEvent(PathContext const& pc)
  {
    log_with_indent(3, "processing path for event: " + pc.pathName());
  }

  void
  Tracer::postPathEvent(PathContext const& pc, HLTPathStatus const&)
  {
    log_with_indent(3, "finished path for event: " + pc.pathName());
  }

  void
  Tracer::preModuleEvent(ModuleContext const& mc)
  {
    ++depth_;
    log_with_indent(3 + depth_, "module for event: " + mc.moduleLabel());
  }

  void
  Tracer::postModuleEvent(ModuleContext const& mc)
  {
    --depth_;
    log_with_indent(4 + depth_, "finished for event: " + mc.moduleLabel());
  }

  void
  art::Tracer::preBeginRun(Run const& run)
  {
    depth_ = 0;
    std::ostringstream msg;
    msg << "processing begin run: " << run.id()
        << " time: " << run.beginTime().value();
    log_with_indent(2, msg.str());
  }

  void
  art::Tracer::postBeginRun(Run const&)
  {
    log_with_indent(2, "finished begin run");
  }

  void
  Tracer::prePathBeginRun(string const& iName)
  {
    log_with_indent(3, "processing path for begin run: " + iName);
  }

  void
  Tracer::postPathBeginRun(string const& /*iName*/, HLTPathStatus const&)
  {
    log_with_indent(3, "finished path for begin run");
  }

  void
  Tracer::preModuleBeginRun(ModuleContext const& mc)
  {
    ++depth_;
    log_with_indent(3 + depth_, "module for begin run: " + mc.moduleLabel());
  }

  void
  Tracer::postModuleBeginRun(ModuleContext const& mc)
  {
    --depth_;
    log_with_indent(4 + depth_, "finished for begin run: " + mc.moduleLabel());
  }

  void
  art::Tracer::preEndRun(RunID const& iID, Timestamp const& iTime)
  {
    depth_ = 0;
    std::ostringstream msg;
    msg << "processing end run: " << iID << " time: " << iTime.value();
    log_with_indent(2, msg.str());
  }

  void
  art::Tracer::postEndRun(Run const&)
  {
    log_with_indent(2, "finished end run");
  }

  void
  Tracer::prePathEndRun(string const& iName)
  {
    log_with_indent(3, "processing path for end run: " + iName);
  }

  void
  Tracer::postPathEndRun(string const& /*iName*/, HLTPathStatus const&)
  {
    log_with_indent(3, "finished path for end run");
  }

  void
  Tracer::preModuleEndRun(ModuleContext const& mc)
  {
    ++depth_;
    log_with_indent(3 + depth_, "module for end run: " + mc.moduleLabel());
  }

  void
  Tracer::postModuleEndRun(ModuleContext const& mc)
  {
    --depth_;
    log_with_indent(4 + depth_, "finished for end run: " + mc.moduleLabel());
  }

  void
  art::Tracer::preBeginSubRun(SubRun const& subRun)
  {
    depth_ = 0;
    std::ostringstream msg;
    msg << "processing begin subRun: " << subRun.id()
        << " time: " << subRun.beginTime().value();
    log_with_indent(2, msg.str());
  }

  void
  art::Tracer::postBeginSubRun(SubRun const&)
  {
    log_with_indent(2, "finished begin subRun");
  }

  void
  Tracer::prePathBeginSubRun(string const& iName)
  {
    log_with_indent(3, "processing path for begin subRun: " + iName);
  }

  void
  Tracer::postPathBeginSubRun(string const& /*iName*/, HLTPathStatus const&)
  {
    log_with_indent(3, "finished path for begin subRun");
  }

  void
  Tracer::preModuleBeginSubRun(ModuleContext const& mc)
  {
    ++depth_;
    log_with_indent(3 + depth_, "module for begin subRun: " + mc.moduleLabel());
  }

  void
  Tracer::postModuleBeginSubRun(ModuleContext const& mc)
  {
    --depth_;
    log_with_indent(4, "finished for begin subRun: " + mc.moduleLabel());
  }

  void
  art::Tracer::preEndSubRun(SubRunID const& iID, Timestamp const& iTime)
  {
    depth_ = 0;
    std::ostringstream msg;
    msg << "processing end subRun: " << iID << " time: " << iTime.value();
    log_with_indent(2, msg.str());
  }

  void
  art::Tracer::postEndSubRun(SubRun const&)
  {
    log_with_indent(2, "finished end subRun");
  }

  void
  Tracer::prePathEndSubRun(string const& iName)
  {
    log_with_indent(3, "processing path for end subRun: " + iName);
  }

  void
  Tracer::postPathEndSubRun(string const& /*iName*/, HLTPathStatus const&)
  {
    log_with_indent(3, "finished path for end subRun");
  }

  void
  Tracer::preModuleEndSubRun(ModuleContext const& mc)
  {
    ++depth_;
    log_with_indent(3 + depth_, "module for end subRun: " + mc.moduleLabel());
  }

  void
  Tracer::postModuleEndSubRun(ModuleContext const& mc)
  {
    --depth_;
    log_with_indent(4 + depth_, "finished for end subRun: " + mc.moduleLabel());
  }

  void
  Tracer::preModuleConstruction(ModuleDescription const& md)
  {
    log_with_indent(1, "constructing module: " + md.moduleLabel());
  }

  void
  Tracer::postModuleConstruction(ModuleDescription const& md)
  {
    log_with_indent(1, "construction finished: " + md.moduleLabel());
  }

  void
  Tracer::preModuleBeginJob(ModuleDescription const& md)
  {
    log_with_indent(1, "beginJob module: " + md.moduleLabel());
  }

  void
  Tracer::postModuleBeginJob(ModuleDescription const& md)
  {
    log_with_indent(1, "beginJob finished: " + md.moduleLabel());
  }

  void
  Tracer::preModuleEndJob(ModuleDescription const& md)
  {
    log_with_indent(1, "endJob module: " + md.moduleLabel());
  }

  void
  Tracer::postModuleEndJob(ModuleDescription const& md)
  {
    log_with_indent(1, "endJob finished: " + md.moduleLabel());
  }

} // namespace art

DECLARE_ART_SERVICE(art::Tracer, SHARED)
DEFINE_ART_SERVICE(art::Tracer)
