// ======================================================================
//
// TimeTracker
//
// ======================================================================

#include "art/Framework/Services/Optional/TimeTracker.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Ntuple/sqlite_helpers.h"
#include "boost/format.hpp"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

using namespace ntuple;
using std::setw;

namespace {
  // 'using' declaration won't work because 'now' is a member function
  auto now = std::bind(&tbb::tick_count::now);
}

// ======================================================================
std::ostream&
art::operator<<(std::ostream& os, art::Statistics const& info)
{
  os << info.label << "  "
     << boost::format(" %=12g ") % info.min
     << boost::format(" %=12g ") % info.mean
     << boost::format(" %=12g ") % info.max
     << boost::format(" %=12g ") % info.median
     << boost::format(" %=12g ") % info.rms
     << boost::format(" %=10d ") % info.n;
  return os;
}

// ======================================================================

art::TimeTracker::TimeTracker(ServiceTable<Config> const & config, ActivityRegistry& iRegistry)
  : printSummary_{config().printSummary()}
  , dbMgr_{config().dbOutput().filename()}
  , overwriteContents_{config().dbOutput().overwrite()}
    // table headers
  , timeReportTuple_{"ReportType","Min","Mean","Max","Median","RMS","nEvts" }
  , timeEventTuple_ {"Run","Subrun","Event","Time" }
  , timeModuleTuple_{"Run","Subrun","Event","PathModuleId","Time"}
    // tables
  , timeReportTable_{dbMgr_, "TimeReport", timeReportTuple_, true} // always recompute reports
  , timeEventTable_ {dbMgr_, "TimeEvent" , timeEventTuple_ , overwriteContents_}
  , timeModuleTable_{dbMgr_, "TimeModule", timeModuleTuple_, overwriteContents_}
{
  iRegistry.sPreProcessPath.watch(this, &TimeTracker::prePathProcessing);

  iRegistry.sPostEndJob.watch(this, &TimeTracker::postEndJob);

  iRegistry.sPreProcessEvent.watch(this, &TimeTracker::preEventProcessing);
  iRegistry.sPostProcessEvent.watch(this, &TimeTracker::postEventProcessing);

  iRegistry.sPreModule.watch(this, &TimeTracker::preModule);
  iRegistry.sPostModule.watch(this, &TimeTracker::postModule);
}

//======================================================================
void
art::TimeTracker::prePathProcessing(std::string const& pathname)
{
  pathname_ = pathname;
}

//======================================================================
void
art::TimeTracker::postEndJob()
{
  timeEventTable_.flush();
  timeModuleTable_.flush();

  auto const evtStats = (timeEventTable_.lastRowid() == 0 ) ? Statistics{} : Statistics{"Full event", dbMgr_, "TimeEvent", "Time"};
  using namespace sqlite;

  result r;
  r << select_distinct("PathModuleId").from(timeModuleTable_);
  std::vector<std::string> modules;
  r >> modules;

  std::vector<Statistics> modStats;
  for (auto const& mod : modules) {
    std::string const ddl =
      "CREATE TABLE temp.tmpModTable AS "s +
      "SELECT * FROM TimeModule WHERE PathModuleId='"s+mod+"'"s;
    sqlite::exec(dbMgr_, ddl);
    modStats.emplace_back(mod, dbMgr_, "temp.tmpModTable", "Time");
    sqlite::dropTable(dbMgr_, "temp.tmpModTable");
  }

  if (dbMgr_.logToDb())
    logToDatabase_(evtStats, modStats);
  if (printSummary_)
    logToDestination_(evtStats, modStats);
}

//======================================================================
void
art::TimeTracker::preEventProcessing(Event const& ev)
{
  eventId_ = ev.id();
  eventStart_ = now();
}

void
art::TimeTracker::postEventProcessing(Event const&)
{
  double const t = (now()-eventStart_).seconds();
  sqlite::insert_into(timeEventTable_).values(eventId_.run(),
                                              eventId_.subRun(),
                                              eventId_.event(),
                                              t);
}

//======================================================================
void
art::TimeTracker::preModule(ModuleDescription const&)
{
  moduleStart_ = now();
}

void
art::TimeTracker::postModule(ModuleDescription const& desc)
{
  double const t = (now()-moduleStart_).seconds();
  sqlite::insert_into(timeModuleTable_).values(eventId_.run(),
                                               eventId_.subRun(),
                                               eventId_.event(),
                                               pathname_+":"s+desc.moduleLabel()+":"s+desc.moduleName(),
                                               t);
}

//======================================================================
void
art::TimeTracker::logToDatabase_(Statistics const& evt,
                                 std::vector<Statistics> const& modules)
{
  if (evt.n == 0u) return;
  sqlite::insert_into(timeReportTable_).values(evt.label,
                                               evt.min,
                                               evt.mean,
                                               evt.max,
                                               evt.median,
                                               evt.rms,
                                               evt.n);
  for (auto const& mod: modules) {
    sqlite::insert_into(timeReportTable_).values(mod.label,
                                                 mod.min,
                                                 mod.mean,
                                                 mod.max,
                                                 mod.median,
                                                 mod.rms,
                                                 mod.n);
  }
}

void
art::TimeTracker::logToDestination_(Statistics const& evt,
                                    std::vector<Statistics> const& modules)
{
  std::size_t width {30};
  cet::for_all(modules, [&width](auto const& mod) { width = std::max(width, mod.label.size()); });

  std::ostringstream msgOss;
  msgOss << std::string(width+4+5*14+12,'=') << "\n";
  msgOss << std::setw(width+2) << std::left << "TimeTracker printout (sec)"
         << boost::format(" %=12s ") % "Min"
         << boost::format(" %=12s ") % "Avg"
         << boost::format(" %=12s ") % "Max"
         << boost::format(" %=12s ") % "Median"
         << boost::format(" %=12s ") % "RMS"
         << boost::format(" %=10s ") % "nEvts" << "\n";

  msgOss << std::string(width+4+5*14+12,'=') << "\n";

  if (evt.n == 0u) {
    msgOss << "[ No processed events ]\n";
  }
  else {
    // N.B. setw(width) applies to the first field in
    //      ostream& art::operator<<(ostream&, Statistics const&).
    msgOss << setw(width) << evt << '\n';
    msgOss << std::string(width+4+5*14+12,'-') << "\n";
    if (modules.empty())
      msgOss << "[ No configured modules ]\n";
    for (auto const& mod : modules) {
      msgOss << setw(width) << mod << '\n';
    }
  }

  msgOss << std::string(width+4+5*14+12,'=') << "\n";
  mf::LogAbsolute("TimeTracker") << msgOss.str();
}

DEFINE_ART_SERVICE(art::TimeTracker)
