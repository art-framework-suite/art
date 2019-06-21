// vim: set sw=2 expandtab :
// ======================================================================
// MemoryTracker
//
// This MemoryTracker implementation is supported only for Linux
// systems.  It relies on the proc file system to record VSize and RSS
// information throughout the course of an art process.  It inserts
// memory information into an in-memory SQLite database, or an
// external file if the user provides a non-empty file name.
//
// Since information that procfs provides is process-specific, the
// MemoryTracker does not attempt to provide per-module information in
// the context of multi-threading.  If more than one thread has been
// enabled for the art process, only the maximum RSS and VSize for the
// process is reported and the end of the job.
// ======================================================================

#ifndef __linux__
#error "This source file can be built only for Linux platforms."
#endif

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Optional/detail/LinuxMallInfo.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "art/Framework/Services/System/DatabaseConnection.h"
#include "art/Persistency/Provenance/ModuleContext.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/PathContext.h"
#include "art/Utilities/Globals.h"
#include "art/Utilities/LinuxProcData.h"
#include "art/Utilities/LinuxProcMgr.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/sqlite/Connection.h"
#include "cetlib/sqlite/Ntuple.h"
#include "cetlib/sqlite/select.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/OptionalAtom.h"
#include "fhiclcpp/types/Sequence.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

using namespace std;
using namespace string_literals;
using namespace cet;

using art::detail::LinuxMallInfo;
using vsize_t = art::LinuxProcData::vsize_t;
using rss_t = art::LinuxProcData::rss_t;

namespace art {

  class MemoryTracker {
    template <unsigned N>
    using name_array = cet::sqlite::name_array<N>;
    using peakUsage_t = cet::sqlite::Ntuple<string, double, string>;
    using otherInfo_t =
      cet::sqlite::Ntuple<string, string, string, double, double>;
    using memEvent_t =
      cet::sqlite::Ntuple<string, uint32_t, uint32_t, uint32_t, double, double>;
    using memModule_t = cet::sqlite::Ntuple<string,
                                            uint32_t,
                                            uint32_t,
                                            uint32_t,
                                            string,
                                            string,
                                            string,
                                            double,
                                            double>;
    using memEventHeap_t = cet::sqlite::Ntuple<string,
                                               uint32_t,
                                               uint32_t,
                                               uint32_t,
                                               int,
                                               int,
                                               int,
                                               int,
                                               int,
                                               int,
                                               int>;
    using memModuleHeap_t = cet::sqlite::Ntuple<string,
                                                uint32_t,
                                                uint32_t,
                                                uint32_t,
                                                string,
                                                string,
                                                string,
                                                int,
                                                int,
                                                int,
                                                int,
                                                int,
                                                int,
                                                int>;

  public:
    static constexpr bool service_handle_allowed{false};

    struct Config {
      template <typename T>
      using Atom = fhicl::Atom<T>;
      using Name = fhicl::Name;
      using Comment = fhicl::Comment;
      template <typename T>
      using Table = fhicl::Table<T>;
      struct DBoutput {
        Atom<string> filename{Name{"filename"}, ""};
        Atom<bool> overwrite{Name{"overwrite"}, false};
      };
      Table<DBoutput> dbOutput{Name{"dbOutput"}};
      Atom<bool> includeMallocInfo{Name{"includeMallocInfo"}, false};
    };

    using Parameters = ServiceTable<Config>;
    MemoryTracker(Parameters const&, ActivityRegistry&);

  private:
    void prePathProcessing(PathContext const& pc);
    void recordOtherData(ModuleDescription const& md, string const& step);
    void recordOtherData(ModuleContext const& mc, string const& step);
    void recordEventData(Event const& e, string const& step);
    void recordModuleData(ModuleContext const& mc, string const& step);
    void postEndJob();
    bool checkMallocConfig_(string const&, bool);
    void recordPeakUsages_();
    void flushTables_();
    void summary_();

    LinuxProcMgr procInfo_{};
    string const fileName_;
    unique_ptr<cet::sqlite::Connection> const db_;
    bool const overwriteContents_;
    bool const includeMallocInfo_;

    // NB: using "current" semantics for the MemoryTracker is valid
    // since per-module/event information are retrieved only in a
    // sequential (i.e. single-threaded) context.
    EventID currentEventID_{EventID::invalidEvent()};
    name_array<3u> peakUsageColumns_{{"Name", "Value", "Description"}};
    name_array<5u> otherInfoColumns_{
      {"Step", "ModuleLabel", "ModuleType", "Vsize", "RSS"}};
    name_array<6u> eventColumns_{
      {"Step", "Run", "SubRun", "Event", "Vsize", "RSS"}};
    name_array<9u> moduleColumns_{{"Step",
                                   "Run",
                                   "SubRun",
                                   "Event",
                                   "Path",
                                   "ModuleLabel",
                                   "ModuleType",
                                   "Vsize",
                                   "RSS"}};
    name_array<11u> eventHeapColumns_{{"Step",
                                       "Run",
                                       "SubRun",
                                       "Event",
                                       "arena",
                                       "ordblks",
                                       "keepcost",
                                       "hblkhd",
                                       "hblks",
                                       "uordblks",
                                       "fordblks"}};
    name_array<14u> moduleHeapColumns_{{"Step",
                                        "Run",
                                        "SubRun",
                                        "Event",
                                        "Path",
                                        "ModuleLabel",
                                        "ModuleType",
                                        "arena",
                                        "ordblks",
                                        "keepcost",
                                        "hblkhd",
                                        "hblks",
                                        "uordblks",
                                        "fordblks"}};
    peakUsage_t peakUsageTable_;
    otherInfo_t otherInfoTable_;
    memEvent_t eventTable_;
    memModule_t moduleTable_;
    unique_ptr<memEventHeap_t> eventHeapTable_;
    unique_ptr<memModuleHeap_t> moduleHeapTable_;
  };

  MemoryTracker::MemoryTracker(ServiceTable<Config> const& config,
                               ActivityRegistry& iReg)
    : fileName_{config().dbOutput().filename()}
    , db_{ServiceHandle<DatabaseConnection>{}->get(fileName_)}
    , overwriteContents_{config().dbOutput().overwrite()}
    , includeMallocInfo_{checkMallocConfig_(config().dbOutput().filename(),
                                            config().includeMallocInfo())}
    // Fix so that a value of 'false' is an error if filename => in-memory db.
    , peakUsageTable_{*db_, "PeakUsage", peakUsageColumns_, true}
    // always recompute the peak usage
    , otherInfoTable_{*db_, "OtherInfo", otherInfoColumns_, overwriteContents_}
    , eventTable_{*db_, "EventInfo", eventColumns_, overwriteContents_}
    , moduleTable_{*db_, "ModuleInfo", moduleColumns_, overwriteContents_}
    , eventHeapTable_{includeMallocInfo_ ?
                        make_unique<memEventHeap_t>(*db_,
                                                    "EventMallocInfo",
                                                    eventHeapColumns_) :
                        nullptr}
    , moduleHeapTable_{includeMallocInfo_ ?
                         make_unique<memModuleHeap_t>(*db_,
                                                      "ModuleMallocInfo",
                                                      moduleHeapColumns_) :
                         nullptr}
  {
    iReg.sPostEndJob.watch(this, &MemoryTracker::postEndJob);
    auto const nthreads = Globals::instance()->nthreads();
    if (nthreads != 1) {
      mf::LogWarning("MemoryTracker")
        << "Since " << nthreads
        << " threads have been configured, only process-level\n"
           "memory usage will be recorded at the end of the job.";
    }

    if (!fileName_.empty() && nthreads == 1u) {
      iReg.sPreModuleConstruction.watch([this](auto const& md) {
        this->recordOtherData(md, "PreModuleConstruction");
      });
      iReg.sPostModuleConstruction.watch([this](auto const& md) {
        this->recordOtherData(md, "PostModuleConstruction");
      });
      iReg.sPreModuleBeginJob.watch(
        [this](auto const& md) { this->recordOtherData(md, "PreBeginJob"); });
      iReg.sPostModuleBeginJob.watch(
        [this](auto const& md) { this->recordOtherData(md, "PostBeginJob"); });
      iReg.sPreModuleBeginRun.watch(
        [this](auto const& mc) { this->recordOtherData(mc, "PreBeginRun"); });
      iReg.sPostModuleBeginRun.watch(
        [this](auto const& mc) { this->recordOtherData(mc, "PostBeginRun"); });
      iReg.sPreModuleBeginSubRun.watch([this](auto const& mc) {
        this->recordOtherData(mc, "PreBeginSubRun");
      });
      iReg.sPostModuleBeginSubRun.watch([this](auto const& mc) {
        this->recordOtherData(mc, "PostBeginSubRun");
      });
      iReg.sPreProcessEvent.watch([this](auto const& e, ScheduleContext) {
        this->recordEventData(e, "PreProcessEvent");
      });
      iReg.sPostProcessEvent.watch([this](auto const& e, ScheduleContext) {
        this->recordEventData(e, "PostProcessEvent");
      });
      iReg.sPreModule.watch([this](auto const& mc) {
        this->recordModuleData(mc, "PreProcessModule");
      });
      iReg.sPostModule.watch([this](auto const& mc) {
        this->recordModuleData(mc, "PostProcessModule");
      });
      iReg.sPreWriteEvent.watch([this](auto const& mc) {
        this->recordModuleData(mc, "PreWriteEvent");
      });
      iReg.sPostWriteEvent.watch([this](auto const& mc) {
        this->recordModuleData(mc, "PostWriteEvent");
      });
      iReg.sPreModuleEndSubRun.watch(
        [this](auto const& mc) { this->recordOtherData(mc, "PreEndSubRun"); });
      iReg.sPreModuleEndRun.watch(
        [this](auto const& mc) { this->recordOtherData(mc, "PreEndRun"); });
      iReg.sPreModuleEndJob.watch(
        [this](auto const& md) { this->recordOtherData(md, "PreEndJob"); });
      iReg.sPostModuleEndSubRun.watch(
        [this](auto const& mc) { this->recordOtherData(mc, "PostEndSubRun"); });
      iReg.sPostModuleEndRun.watch(
        [this](auto const& mc) { this->recordOtherData(mc, "PostEndRun"); });
      iReg.sPostModuleEndJob.watch(
        [this](auto const& md) { this->recordOtherData(md, "PostEndJob"); });
    }
  }

  void
  MemoryTracker::recordOtherData(ModuleContext const& mc, string const& step)
  {
    recordOtherData(mc.moduleDescription(), step);
  }

  void
  MemoryTracker::recordOtherData(ModuleDescription const& md,
                                 string const& step)
  {
    auto const data = procInfo_.getCurrentData();
    otherInfoTable_.insert(step,
                           md.moduleLabel(),
                           md.moduleName(),
                           LinuxProcData::getValueInMB<vsize_t>(data),
                           LinuxProcData::getValueInMB<rss_t>(data));
  }

  void
  MemoryTracker::recordEventData(Event const& e, string const& step)
  {
    currentEventID_ = e.id();
    auto const currentMemory = procInfo_.getCurrentData();
    eventTable_.insert(step,
                       currentEventID_.run(),
                       currentEventID_.subRun(),
                       currentEventID_.event(),
                       LinuxProcData::getValueInMB<vsize_t>(currentMemory),
                       LinuxProcData::getValueInMB<rss_t>(currentMemory));
    if (includeMallocInfo_) {
      auto minfo = LinuxMallInfo{}.get();
      eventHeapTable_->insert(step,
                              currentEventID_.run(),
                              currentEventID_.subRun(),
                              currentEventID_.event(),
                              minfo.arena,
                              minfo.ordblks,
                              minfo.keepcost,
                              minfo.hblkhd,
                              minfo.hblks,
                              minfo.uordblks,
                              minfo.fordblks);
    }
  }

  void
  MemoryTracker::recordModuleData(ModuleContext const& mc, string const& step)
  {
    auto const currentMemory = procInfo_.getCurrentData();
    moduleTable_.insert(step,
                        currentEventID_.run(),
                        currentEventID_.subRun(),
                        currentEventID_.event(),
                        mc.pathName(),
                        mc.moduleLabel(),
                        mc.moduleName(),
                        LinuxProcData::getValueInMB<vsize_t>(currentMemory),
                        LinuxProcData::getValueInMB<rss_t>(currentMemory));
    if (includeMallocInfo_) {
      auto minfo = LinuxMallInfo{}.get();
      moduleHeapTable_->insert(step,
                               currentEventID_.run(),
                               currentEventID_.subRun(),
                               currentEventID_.event(),
                               mc.pathName(),
                               mc.moduleLabel(),
                               mc.moduleName(),
                               minfo.arena,
                               minfo.ordblks,
                               minfo.keepcost,
                               minfo.hblkhd,
                               minfo.hblks,
                               minfo.uordblks,
                               minfo.fordblks);
    }
  }

  void
  MemoryTracker::postEndJob()
  {
    recordPeakUsages_();
    flushTables_();
    summary_();
  }

  bool
  MemoryTracker::checkMallocConfig_(string const& dbfilename,
                                    bool const include)
  {
    if (include && dbfilename.empty()) {
      string const errmsg =
        "\n'includeMallocInfo : true' is valid only if a nonempty db filename is specified:\n\n"s +
        "   MemoryTracker: {\n"
        "      includeMallocInfo: true\n"
        "      dbOutput: {\n"
        "         filename: \"your_filename.db\"\n"
        "      }\n"
        "   }\n\n";
      throw Exception{errors::Configuration} << errmsg;
    }
    return include;
  }

  void
  MemoryTracker::recordPeakUsages_()
  {
    peakUsageTable_.insert(
      "VmPeak", procInfo_.getVmPeak(), "Peak virtual memory (MB)");
    peakUsageTable_.insert(
      "VmHWM", procInfo_.getVmHWM(), "Peak resident set size (MB)");
  }

  void
  MemoryTracker::flushTables_()
  {
    otherInfoTable_.flush();
    eventTable_.flush();
    moduleTable_.flush();
    peakUsageTable_.flush();
    if (eventHeapTable_) {
      eventHeapTable_->flush();
    }
    if (moduleHeapTable_) {
      moduleHeapTable_->flush();
    }
  }

  void
  MemoryTracker::summary_()
  {
    using namespace cet::sqlite;
    using namespace std;
    query_result<double> rVMax;
    query_result<double> rRMax;
    rVMax << select("Value")
               .from(*db_, peakUsageTable_.name())
               .where("Name='VmPeak'");
    rRMax << select("Value")
               .from(*db_, peakUsageTable_.name())
               .where("Name='VmHWM'");
    mf::LogAbsolute log{"MemoryTracker"};
    HorizontalRule const rule{100};
    log << '\n' << rule('=') << '\n';
    log << std::left << "MemoryTracker summary (base-10 MB units used)\n\n";
    log << "  Peak virtual memory usage (VmPeak)  : " << unique_value(rVMax)
        << " MB\n"
        << "  Peak resident set size usage (VmHWM): " << unique_value(rRMax)
        << " MB\n";
    if (!(fileName_.empty() || fileName_ == ":memory:")) {
      log << "  Details saved in: '" << fileName_ << "'\n";
    }
    log << rule('=');
  }

} // namespace art

DECLARE_ART_SERVICE(art::MemoryTracker, SHARED)
DEFINE_ART_SERVICE(art::MemoryTracker)
