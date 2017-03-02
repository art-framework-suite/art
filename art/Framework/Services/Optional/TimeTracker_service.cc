// ======================================================================
//
// TimeTracker
//
// ======================================================================

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "boost/format.hpp"
#include "cetlib/Ntuple/Ntuple.h"
#include "cetlib/Ntuple/sqlite_DBmanager.h"
#include "cetlib/Ntuple/sqlite_helpers.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Name.h"
#include "fhiclcpp/types/Table.h"
#include "tbb/tick_count.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

using namespace ntuple;
using std::setw;

namespace {

  // 'using' declaration won't work because 'now' is a member function
  auto now = std::bind(&tbb::tick_count::now);

  struct Statistics {

    explicit Statistics() = default;

    explicit Statistics(std::string const& p,
                        std::string const& label,
                        std::string const& type,
                        sqlite3* db,
                        std::string const& table,
                        std::string const& column)
      : path{p}
      , mod_label{label}
      , mod_type{type}
      , min{sqlite::min(db, table, column)}
      , mean{sqlite::mean(db, table, column)}
      , max{sqlite::max(db, table, column)}
      , median{sqlite::median(db, table, column)}
      , rms{sqlite::rms(db, table, column)}
      , n{sqlite::nrows(db, table)}
    {}

    std::string path {};
    std::string mod_label {};
    std::string mod_type {};
    double min {-1.};
    double mean {-1.};
    double max {-1.};
    double median {-1.};
    double rms {-1.};
    unsigned n {0u};
  };

  std::ostream& operator<<(std::ostream& os, Statistics const& info)
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
}

namespace art {
  class TimeTracker;
}

// =======================================================================
class art::TimeTracker {
public:

  struct Config {
    fhicl::Atom<bool> printSummary { fhicl::Name("printSummary"), true };

    struct DBoutput {
      fhicl::Atom<std::string> filename { fhicl::Name("filename"), "" };
      fhicl::Atom<bool> overwrite { fhicl::Name("overwrite"), false };
    };
    fhicl::Table<DBoutput> dbOutput { fhicl::Name("dbOutput") };
  };

  using Parameters = ServiceTable<Config>;
  TimeTracker(ServiceTable<Config> const&, ActivityRegistry&);

private:

  void prePathProcessing(std::string const&);

  void postEndJob();

  void preEventProcessing(Event const&);
  void postEventProcessing(Event const&);

  void startTime(ModuleDescription const&);
  void recordTime(ModuleDescription const& md, std::string const& suffix);

  void logToDatabase_(Statistics const& evt, std::vector<Statistics> const& modules);
  void logToDestination_(Statistics const& evt, std::vector<Statistics> const& modules);

  std::string pathname_;
  EventID eventId_;
  tbb::tick_count eventStart_;
  tbb::tick_count moduleStart_;

  bool printSummary_;
  sqlite::DBmanager dbMgr_;
  bool overwriteContents_;

  template<unsigned SIZE>
  using name_array = sqlite::name_array<SIZE>;
  name_array<4u> timeEventTuple_;
  name_array<7u> timeModuleTuple_;

  using timeEvent_t  = ntuple::Ntuple<uint32_t,uint32_t,uint32_t,double>;
  using timeModule_t = ntuple::Ntuple<uint32_t,uint32_t,uint32_t,std::string,std::string,std::string,double>;

  timeEvent_t  timeEventTable_;
  timeModule_t timeModuleTable_;
};  // art::TimeTracker


//=================================================================
// Implementation below

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

  iRegistry.sPreModule.watch(this, &TimeTracker::startTime);
  iRegistry.sPostModule.watch([this](auto const& md) { this->recordTime(md,""s); });
  iRegistry.sPreWriteEvent.watch(this, &TimeTracker::startTime);
  iRegistry.sPostWriteEvent.watch([this](auto const& md) { this->recordTime(md,"(write)"s); });
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
  timeEventTable_.insert(eventId_.run(),
                                              eventId_.subRun(),
                                              eventId_.event(),
                                              t);
}

//======================================================================
void
art::TimeTracker::startTime(ModuleDescription const&)
{
  moduleStart_ = now();
}

void
art::TimeTracker::recordTime(ModuleDescription const& desc, std::string const& suffix)
{
  double const t = (now()-moduleStart_).seconds();
  timeModuleTable_.insert(eventId_.run(),
                                               eventId_.subRun(),
                                               eventId_.event(),
                                               pathname_,
                                               desc.moduleLabel(),
                                               desc.moduleName()+suffix,
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

//===========================================================
DECLARE_ART_SERVICE(art::TimeTracker, LEGACY)
DEFINE_ART_SERVICE(art::TimeTracker)
