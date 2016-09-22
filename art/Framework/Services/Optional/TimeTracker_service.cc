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
  std::string label {info.path};
  if (info.path != "Full event")
    label += ":"s + info.mod_label + ":"s + info.mod_type;
  os << label << "  "
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
  , timeEventTuple_ {"Run","SubRun","Event","Time" }
  , timeModuleTuple_{"Run","SubRun","Event","Path", "ModuleLabel", "ModuleType","Time"}
    // tables
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

  if (!printSummary_) return;

  if (timeEventTable_.lastRowid() == 0 && timeModuleTable_.lastRowid() == 0) {
    logToDestination_(Statistics{}, std::vector<Statistics>{});
    return;
  }

  if (timeEventTable_.lastRowid() == 0 && timeModuleTable_.lastRowid() != 0) {
    throw art::Exception(art::errors::LogicError)
      << "Malformed TimeTracker database.  The TimeEvent table is empty, but\n"
      << "the TimeModule table is not.  Please contact artists@fnal.gov.";
  }

  // Gather statistics for full Event
  // -- Unfortunately, this is not a simple query since the
  //    'RootOutput(write)' times are not recorded in the TimeEvent
  //    rows.  They must be added in.

  std::string const fullEventTime_ddl =
    "CREATE TABLE temp.fullEventTime AS "
    "SELECT Run,Subrun,Event,SUM(Time) AS FullEventTime FROM ("
    "       SELECT Run,Subrun,Event,Time FROM TimeEvent"
    "       UNION"
    "       SELECT Run,Subrun,Event,Time FROM TimeModule WHERE ModuleType='RootOutput(write)'"
    ") GROUP BY Run,Subrun,Event";
  sqlite::exec(dbMgr_, fullEventTime_ddl);
  Statistics const evtStats {"Full event", "", "", dbMgr_, "temp.fullEventTime", "FullEventTime"};
  sqlite::dropTable(dbMgr_, "temp.fullEventTime");

  sqlite::result r;
  r << sqlite::select_distinct("Path","ModuleLabel","ModuleType").from(timeModuleTable_);

  std::vector<Statistics> modStats;
  for (auto& row : r) {
    std::string path {};
    std::string mod_label {};
    std::string mod_type {};
    row >> path >> mod_label >> mod_type;
    std::string const ddl =
      "CREATE TABLE temp.tmpModTable AS "s +
      "SELECT * FROM TimeModule WHERE Path='"s+path+"'"s +
      " AND ModuleLabel='"s+mod_label+"'"s +
      " AND ModuleType='"s+mod_type+"'"s;
    sqlite::exec(dbMgr_, ddl);
    modStats.emplace_back(path, mod_label, mod_type, dbMgr_, "temp.tmpModTable", "Time");
    sqlite::dropTable(dbMgr_, "temp.tmpModTable");
  }

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
                                               pathname_,
                                               desc.moduleLabel(),
                                               desc.moduleName(),
                                               t);
}

//======================================================================
void
art::TimeTracker::logToDestination_(Statistics const& evt,
                                    std::vector<Statistics> const& modules)
{
  std::size_t width {30};
  auto identifier_size = [](Statistics const& s) {
    return s.path.size() + s.mod_label.size() + s.mod_type.size() + 2; // Don't forget the two ':'s.
  };
  cet::for_all(modules, [&identifier_size,&width](auto const& mod) { width = std::max(width, identifier_size(mod)); });

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
