// ======================================================================
//
// MemoryTracker
//
// The MemoryTracker service records VSize and RSS information
// throughout the course of an art process.  It inserts memory
// information into an in-memory SQLite database, or an external file
// if the user provides a non-empty file name.
//
// In the context of multi-threading, the memory information recorded
// corresponds to all memory information for the process, and not for
// individual threads.  A consequence of this is that the recorded
// memory usage for a given event may not correspond to memory usage
// of that event per se, but can include contributions from other
// events that are being processed concurrently.
//
// In order to have a straightforward interpretation of the
// per-event/module memory usage of an art process, then only one
// thread should be used.  The max VSize and RSS measurements of a job
// should be meaningful, however, even in a multi-threaded process.
//
// ======================================================================

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Optional/detail/LinuxMallInfo.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "art/Framework/Services/System/DatabaseConnection.h"
#include "art/Utilities/Globals.h"
#include "art/Utilities/LinuxProcData.h"
#include "art/Utilities/LinuxProcMgr.h"
#include "art/Utilities/PerThread.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
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

#include <memory>
#include <sstream>
#include <tuple>

namespace art {

  class MemoryTracker {
  public:
    struct Config {
      using Name = fhicl::Name;
      using Comment = fhicl::Comment;
      struct DBoutput {
        fhicl::Atom<std::string> filename{Name{"filename"}, ""};
        fhicl::Atom<bool> overwrite{Name{"overwrite"}, false};
      };
      fhicl::Table<DBoutput> dbOutput{Name{"dbOutput"}};
      fhicl::Atom<bool> includeMallocInfo{Name{"includeMallocInfo"}, false};
    };

    using Parameters = ServiceTable<Config>;
    MemoryTracker(ServiceTable<Config> const&, ActivityRegistry&);

  private:
    // Callbacks
    // ... Path level
    void prePathProcessing(std::string const&);

    void recordOtherData(ModuleDescription const& md, std::string const& step);
    void recordEventData(Event const& e, std::string const& step);
    void recordModuleData(ModuleDescription const& md, std::string const& step);

    // ... Wrap up
    void postEndJob();

    bool checkMallocConfig_(std::string const&, bool);

    void recordPeakUsages_();
    void flushTables_();
    void summary_();

    LinuxProcMgr procInfo_;
    std::string fileName_;

    // Options
    cet::sqlite::Connection db_;
    bool overwriteContents_;
    bool includeMallocInfo_;

    struct PerScheduleData {
      std::string pathName{};
      art::EventID eventID{};
    };
    std::vector<PerScheduleData> data_;

    template <unsigned N>
    using name_array = cet::sqlite::name_array<N>;

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

    using peakUsage_t = cet::sqlite::Ntuple<std::string, double, std::string>;
    using otherInfo_t = cet::sqlite::
      Ntuple<std::string, std::string, std::string, double, double>;
    using memEvent_t = cet::sqlite::
      Ntuple<std::string, uint32_t, uint32_t, uint32_t, double, double>;
    using memModule_t = cet::sqlite::Ntuple<std::string,
                                            uint32_t,
                                            uint32_t,
                                            uint32_t,
                                            std::string,
                                            std::string,
                                            std::string,
                                            double,
                                            double>;
    using memEventHeap_t = cet::sqlite::Ntuple<std::string,
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
    using memModuleHeap_t = cet::sqlite::Ntuple<std::string,
                                                uint32_t,
                                                uint32_t,
                                                uint32_t,
                                                std::string,
                                                std::string,
                                                std::string,
                                                int,
                                                int,
                                                int,
                                                int,
                                                int,
                                                int,
                                                int>;

    peakUsage_t peakUsageTable_;
    otherInfo_t otherInfoTable_;
    memEvent_t eventTable_;
    memModule_t moduleTable_;
    std::unique_ptr<memEventHeap_t> eventHeapTable_;
    std::unique_ptr<memModuleHeap_t> moduleHeapTable_;
  }; // MemoryTracker

} // namespace art

//====================================================
// Implementation below

using namespace std::string_literals;
using namespace cet;

//======================================================================================
using art::detail::LinuxMallInfo;
using vsize_t = art::LinuxProcData::vsize_t;
using rss_t = art::LinuxProcData::rss_t;

art::MemoryTracker::MemoryTracker(ServiceTable<Config> const& config,
                                  ActivityRegistry& iReg)
  : procInfo_{static_cast<unsigned short>(Globals::instance()->nschedules())}
  , fileName_{config().dbOutput().filename()}
  , db_{ServiceHandle<DatabaseConnection>{}->get(fileName_)}
  , overwriteContents_{config().dbOutput().overwrite()}
  // Fix so that a value of 'false' is an error if filename => in-memory db.
  , includeMallocInfo_{checkMallocConfig_(config().dbOutput().filename(),
                                          config().includeMallocInfo())}
  // tables
  , peakUsageTable_{db_, "PeakUsage", peakUsageColumns_, true}
  // always recompute the peak usage
  , otherInfoTable_{db_, "OtherInfo", otherInfoColumns_, overwriteContents_}
  , eventTable_{db_, "EventInfo", eventColumns_, overwriteContents_}
  , moduleTable_{db_, "ModuleInfo", moduleColumns_, overwriteContents_}
  , eventHeapTable_{includeMallocInfo_ ?
                      std::make_unique<memEventHeap_t>(db_,
                                                       "EventMallocInfo",
                                                       eventHeapColumns_) :
                      nullptr}
  , moduleHeapTable_{includeMallocInfo_ ?
                       std::make_unique<memModuleHeap_t>(db_,
                                                         "ModuleMallocInfo",
                                                         moduleHeapColumns_) :
                       nullptr}
{
  data_.resize(Globals::instance()->nschedules());

  iReg.sPostEndJob.watch(this, &MemoryTracker::postEndJob);

  if (!fileName_.empty()) {
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
      [this](auto const& md) { this->recordOtherData(md, "PreBeginRun"); });
    iReg.sPostModuleBeginRun.watch(
      [this](auto const& md) { this->recordOtherData(md, "PostBeginRun"); });
    iReg.sPreModuleBeginSubRun.watch(
      [this](auto const& md) { this->recordOtherData(md, "PreBeginSubRun"); });
    iReg.sPostModuleBeginSubRun.watch(
      [this](auto const& md) { this->recordOtherData(md, "PostBeginSubRun"); });
    iReg.sPreProcessPath.watch(this, &MemoryTracker::prePathProcessing);
    iReg.sPreProcessEvent.watch(
      [this](auto const& e) { this->recordEventData(e, "PreProcessEvent"); });
    iReg.sPostProcessEvent.watch(
      [this](auto const& e) { this->recordEventData(e, "PostProcessEvent"); });
    iReg.sPreModule.watch([this](auto const& md) {
      this->recordModuleData(md, "PreProcessModule");
    });
    iReg.sPostModule.watch([this](auto const& md) {
      this->recordModuleData(md, "PostProcessModule");
    });
    iReg.sPreWriteEvent.watch(
      [this](auto const& md) { this->recordModuleData(md, "PreWriteEvent"); });
    iReg.sPostWriteEvent.watch(
      [this](auto const& md) { this->recordModuleData(md, "PostWriteEvent"); });
    iReg.sPreModuleEndSubRun.watch(
      [this](auto const& md) { this->recordOtherData(md, "PreEndSubRun"); });
    iReg.sPreModuleEndRun.watch(
      [this](auto const& md) { this->recordOtherData(md, "PreEndRun"); });
    iReg.sPreModuleEndJob.watch(
      [this](auto const& md) { this->recordOtherData(md, "PreEndJob"); });
    iReg.sPostModuleEndSubRun.watch(
      [this](auto const& md) { this->recordOtherData(md, "PostEndSubRun"); });
    iReg.sPostModuleEndRun.watch(
      [this](auto const& md) { this->recordOtherData(md, "PostEndRun"); });
    iReg.sPostModuleEndJob.watch(
      [this](auto const& md) { this->recordOtherData(md, "PostEndJob"); });
  }
}

//======================================================================
void
art::MemoryTracker::prePathProcessing(std::string const& pathname)
{
  auto const sid = PerThread::instance()->getCPC().scheduleID();
  data_[sid.id()].pathName = pathname;
}

//======================================================================
void
art::MemoryTracker::recordOtherData(ModuleDescription const& md,
                                    std::string const& step)
{
  auto const sid = PerThread::instance()->getCPC().scheduleID();
  auto const data = procInfo_.getCurrentData(sid.id());
  otherInfoTable_.insert(step,
                         md.moduleLabel(),
                         md.moduleName(),
                         LinuxProcData::getValueInMB<vsize_t>(data),
                         LinuxProcData::getValueInMB<rss_t>(data));
}

//======================================================================
void
art::MemoryTracker::recordEventData(Event const& e, std::string const& step)
{
  auto const sid = PerThread::instance()->getCPC().scheduleID();
  auto& d = data_[sid.id()];
  d.eventID = e.id();

  auto const currentMemory = procInfo_.getCurrentData(sid.id());

  eventTable_.insert(step,
                     d.eventID.run(),
                     d.eventID.subRun(),
                     d.eventID.event(),
                     LinuxProcData::getValueInMB<vsize_t>(currentMemory),
                     LinuxProcData::getValueInMB<rss_t>(currentMemory));

  if (includeMallocInfo_) {
    auto minfo = LinuxMallInfo{}.get();
    eventHeapTable_->insert(step,
                            d.eventID.run(),
                            d.eventID.subRun(),
                            d.eventID.event(),
                            minfo.arena,
                            minfo.ordblks,
                            minfo.keepcost,
                            minfo.hblkhd,
                            minfo.hblks,
                            minfo.uordblks,
                            minfo.fordblks);
  }
}

//======================================================================
void
art::MemoryTracker::recordModuleData(ModuleDescription const& md,
                                     std::string const& step)
{
  auto const sid = PerThread::instance()->getCPC().scheduleID();
  auto& d = data_[sid.id()];

  auto const currentMemory = procInfo_.getCurrentData(sid.id());

  moduleTable_.insert(step,
                      d.eventID.run(),
                      d.eventID.subRun(),
                      d.eventID.event(),
                      d.pathName,
                      md.moduleLabel(),
                      md.moduleName(),
                      LinuxProcData::getValueInMB<vsize_t>(currentMemory),
                      LinuxProcData::getValueInMB<rss_t>(currentMemory));

  if (includeMallocInfo_) {
    auto minfo = LinuxMallInfo{}.get();
    moduleHeapTable_->insert(step,
                             d.eventID.run(),
                             d.eventID.subRun(),
                             d.eventID.event(),
                             d.pathName,
                             md.moduleLabel(),
                             md.moduleName(),
                             minfo.arena,
                             minfo.ordblks,
                             minfo.keepcost,
                             minfo.hblkhd,
                             minfo.hblks,
                             minfo.uordblks,
                             minfo.fordblks);
  }
}

//======================================================================
void
art::MemoryTracker::postEndJob()
{
  recordPeakUsages_();
  flushTables_();
  summary_();
}

//======================================================================
bool
art::MemoryTracker::checkMallocConfig_(std::string const& dbfilename,
                                       bool const include)
{
  if (include && dbfilename.empty()) {
    std::string const errmsg =
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

//======================================================================
void
art::MemoryTracker::recordPeakUsages_()
{
  peakUsageTable_.insert(
    "VmPeak", procInfo_.getVmPeak(), "Peak virtual memory (MB)");
  peakUsageTable_.insert(
    "VmHWM", procInfo_.getVmHWM(), "Peak resident set size (MB)");
}

//======================================================================
void
art::MemoryTracker::flushTables_()
{
  otherInfoTable_.flush();
  eventTable_.flush();
  moduleTable_.flush();
  peakUsageTable_.flush();
  if (eventHeapTable_)
    eventHeapTable_->flush();
  if (moduleHeapTable_)
    moduleHeapTable_->flush();
}

//======================================================================
void
art::MemoryTracker::summary_()
{
  using namespace cet::sqlite;
  using namespace std;
  query_result<double> rVMax;
  query_result<double> rRMax;
  rVMax
    << select("Value").from(db_, peakUsageTable_.name()).where("Name='VmPeak'");
  rRMax
    << select("Value").from(db_, peakUsageTable_.name()).where("Name='VmHWM'");

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

DECLARE_ART_SERVICE(art::MemoryTracker, LEGACY)
DEFINE_ART_SERVICE(art::MemoryTracker)
