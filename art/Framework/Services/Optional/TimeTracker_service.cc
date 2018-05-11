// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "art/Framework/Services/System/DatabaseConnection.h"
#include "art/Persistency/Provenance/ModuleContext.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/PathContext.h"
#include "art/Utilities/Globals.h"
#include "art/Utilities/PerScheduleContainer.h"
#include "art/Utilities/ScheduleID.h"
#include "boost/format.hpp"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/sqlite/Connection.h"
#include "cetlib/sqlite/Ntuple.h"
#include "cetlib/sqlite/helpers.h"
#include "cetlib/sqlite/statistics.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Name.h"
#include "fhiclcpp/types/Table.h"
#include "hep_concurrency/RecursiveMutex.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "tbb/concurrent_unordered_map.h"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace cet;
using namespace hep::concurrency;

using chrono::steady_clock;

namespace art {

  namespace {

    using ConcurrentKey = std::pair<ScheduleID, std::string>;
    auto
    key(ScheduleID const sid)
    {
      return ConcurrentKey{sid, {}};
    }
    auto
    key(ModuleContext const& mc)
    {
      return ConcurrentKey{mc.scheduleID(), mc.moduleLabel()};
    }

    auto now = bind(&steady_clock::now);

    struct Statistics {
      explicit Statistics() = default;

      explicit Statistics(string const& p,
                          string const& label,
                          string const& type,
                          sqlite3* const db,
                          string const& table,
                          string const& column)
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

      string path{};
      string mod_label{};
      string mod_type{};
      double min{-1.};
      double mean{-1.};
      double max{-1.};
      double median{-1.};
      double rms{-1.};
      unsigned n{0u};
    };

    ostream&
    operator<<(ostream& os, Statistics const& info)
    {
      string label{info.path};
      if (!info.mod_label.empty()) {
        label += ':' + info.mod_label;
      }
      if (!info.mod_type.empty()) {
        label += ':' + info.mod_type;
      }
      os << label << "  " << boost::format(" %=12g ") % info.min
         << boost::format(" %=12g ") % info.mean
         << boost::format(" %=12g ") % info.max
         << boost::format(" %=12g ") % info.median
         << boost::format(" %=12g ") % info.rms
         << boost::format(" %=10d ") % info.n;
      return os;
    }

  } // unnamed namespace

  class TimeTracker {
  public:
    struct Config {
      fhicl::Atom<bool> printSummary{fhicl::Name{"printSummary"}, true};
      struct DBoutput {
        fhicl::Atom<string> filename{fhicl::Name{"filename"}, ""};
        fhicl::Atom<bool> overwrite{fhicl::Name{"overwrite"}, false};
      };
      fhicl::Table<DBoutput> dbOutput{fhicl::Name{"dbOutput"}};
    };
    using Parameters = ServiceTable<Config>;
    explicit TimeTracker(Parameters const&, ActivityRegistry&);

  private:
    struct PerScheduleData {
      EventID eventID;
      steady_clock::time_point eventStart;
      steady_clock::time_point moduleStart;
    };
    template <unsigned SIZE>
    using name_array = cet::sqlite::name_array<SIZE>;
    using timeSource_t =
      cet::sqlite::Ntuple<uint32_t, uint32_t, uint32_t, string, double>;
    using timeEvent_t =
      cet::sqlite::Ntuple<uint32_t, uint32_t, uint32_t, double>;
    using timeModule_t = cet::sqlite::
      Ntuple<uint32_t, uint32_t, uint32_t, string, string, string, double>;

    void postSourceConstruction(ModuleDescription const&);
    void postEndJob();
    void preEventReading(ScheduleID);
    void postEventReading(Event const&, ScheduleID);
    void preEventProcessing(Event const&, ScheduleID);
    void postEventProcessing(Event const&, ScheduleID);
    void startTime(ModuleContext const& mc);
    void recordTime(ModuleContext const& mc, string const& suffix);
    void logToDestination_(Statistics const& evt,
                           vector<Statistics> const& modules);

    tbb::concurrent_unordered_map<ConcurrentKey, PerScheduleData> data_;
    bool const printSummary_;
    unique_ptr<cet::sqlite::Connection> const db_;
    bool const overwriteContents_;
    string sourceType_{};
    name_array<5u> const timeSourceColumnNames_;
    name_array<4u> const timeEventColumnNames_;
    name_array<7u> const timeModuleColumnNames_;
    timeSource_t timeSourceTable_;
    timeEvent_t timeEventTable_;
    timeModule_t timeModuleTable_;
  };

  TimeTracker::TimeTracker(Parameters const& config, ActivityRegistry& areg)
    : printSummary_{config().printSummary()}
    , db_{ServiceHandle<DatabaseConnection>{}->get(
        config().dbOutput().filename())}
    , overwriteContents_{config().dbOutput().overwrite()}
    , timeSourceColumnNames_{{"Run", "SubRun", "Event", "Source", "Time"}}
    , timeEventColumnNames_{{"Run", "SubRun", "Event", "Time"}}
    , timeModuleColumnNames_{{"Run",
                              "SubRun",
                              "Event",
                              "Path",
                              "ModuleLabel",
                              "ModuleType",
                              "Time"}}
    , timeSourceTable_{*db_,
                       "TimeSource",
                       timeSourceColumnNames_,
                       overwriteContents_}
    , timeEventTable_{*db_,
                      "TimeEvent",
                      timeEventColumnNames_,
                      overwriteContents_}
    , timeModuleTable_{*db_,
                       "TimeModule",
                       timeModuleColumnNames_,
                       overwriteContents_}
  {
    areg.sPostSourceConstruction.watch(this,
                                       &TimeTracker::postSourceConstruction);
    areg.sPostEndJob.watch(this, &TimeTracker::postEndJob);
    // Event reading
    areg.sPreSourceEvent.watch(this, &TimeTracker::preEventReading);
    areg.sPostSourceEvent.watch(this, &TimeTracker::postEventReading);
    // Event execution
    areg.sPreProcessEvent.watch(this, &TimeTracker::preEventProcessing);
    areg.sPostProcessEvent.watch(this, &TimeTracker::postEventProcessing);
    // Module execution
    areg.sPreModule.watch(this, &TimeTracker::startTime);
    areg.sPostModule.watch(
      [this](auto const& mc) { this->recordTime(mc, ""s); });
    areg.sPreWriteEvent.watch(this, &TimeTracker::startTime);
    areg.sPostWriteEvent.watch(
      [this](auto const& mc) { this->recordTime(mc, "(write)"s); });
  }

  void
  TimeTracker::postEndJob()
  {
    timeSourceTable_.flush();
    timeEventTable_.flush();
    timeModuleTable_.flush();
    if (!printSummary_) {
      return;
    }
    using namespace cet::sqlite;
    query_result<size_t> rEvents;
    rEvents << select("count(*)").from(*db_, timeEventTable_.name());
    query_result<size_t> rModules;
    rModules << select("count(*)").from(*db_, timeModuleTable_.name());
    auto const nEventRows = unique_value(rEvents);
    auto const nModuleRows = unique_value(rModules);
    if ((nEventRows == 0) && (nModuleRows == 0)) {
      logToDestination_(Statistics{}, vector<Statistics>{});
      return;
    }
    if (nEventRows == 0 && nModuleRows != 0) {
      string const errMsg{
        "Malformed TimeTracker database.  The TimeEvent table is empty, but\n"
        "the TimeModule table is not.  This can happen if an exception has\n"
        "been thrown from a module while processing the first event.  Any\n"
        "saved database file is suspect and should not be used."};
      mf::LogAbsolute("TimeTracker") << errMsg;
      return;
    }
    // Gather statistics for full Event
    // -- Unfortunately, this is not a simple query since the (e.g.)
    //    'RootOutput(write)' times and the source time are not
    //    recorded in the TimeEvent rows.  They must be added in.
    string const fullEventTime_ddl =
      "CREATE TABLE temp.fullEventTime AS "
      "SELECT Run,Subrun,Event,SUM(Time) AS FullEventTime FROM ("
      "       SELECT Run,Subrun,Event,Time FROM TimeEvent"
      "       UNION"
      "       SELECT Run,Subrun,Event,Time FROM TimeModule WHERE ModuleType "
      "LIKE '%(write)'"
      "       UNION"
      "       SELECT Run,Subrun,Event,Time FROM TimeSource"
      ") GROUP BY Run,Subrun,Event";
    using namespace cet::sqlite;
    exec(*db_, fullEventTime_ddl);
    Statistics const evtStats{
      "Full event", "", "", *db_, "temp.fullEventTime", "FullEventTime"};
    drop_table(*db_, "temp.fullEventTime");
    query_result<string, string, string> r;
    r << select_distinct("Path", "ModuleLabel", "ModuleType")
           .from(*db_, timeModuleTable_.name());
    vector<Statistics> modStats;
    modStats.emplace_back(
      "source", sourceType_ + "(read)", "", *db_, "TimeSource", "Time");
    for (auto const& row : r) {
      string path, mod_label, mod_type;
      tie(path, mod_label, mod_type) = row;
      create_table_as("temp.tmpModTable",
                      select("*")
                        .from(*db_, "TimeModule")
                        .where("Path='"s + path + "'"s + " AND ModuleLabel='"s +
                               mod_label + "'"s + " AND ModuleType='"s +
                               mod_type + "'"s));
      modStats.emplace_back(
        path, mod_label, mod_type, *db_, "temp.tmpModTable", "Time");
      drop_table(*db_, "temp.tmpModTable");
    }
    logToDestination_(evtStats, modStats);
  }

  void
  TimeTracker::postSourceConstruction(ModuleDescription const& md)
  {
    sourceType_ = md.moduleName();
  }

  void
  TimeTracker::preEventReading(ScheduleID const sid)
  {
    auto& d = data_[key(sid)];
    d.eventID = EventID::invalidEvent();
    d.eventStart = now();
  }

  void
  TimeTracker::postEventReading(Event const& e, ScheduleID const sid)
  {
    auto& d = data_[key(sid)];
    d.eventID = e.id();
    auto const t = chrono::duration<double>{now() - d.eventStart}.count();
    timeSourceTable_.insert(
      d.eventID.run(), d.eventID.subRun(), d.eventID.event(), sourceType_, t);
  }

  void
  TimeTracker::preEventProcessing(Event const& e[[gnu::unused]],
                                  ScheduleID const sid)
  {
    auto& d = data_[key(sid)];
    assert(d.eventID == e.id());
    d.eventStart = now();
  }

  void
  TimeTracker::postEventProcessing(Event const&, ScheduleID const sid)
  {
    auto const& d = data_[key(sid)];
    auto const t = chrono::duration<double>{now() - d.eventStart}.count();
    timeEventTable_.insert(
      d.eventID.run(), d.eventID.subRun(), d.eventID.event(), t);
  }

  void
  TimeTracker::startTime(ModuleContext const& mc)
  {
    data_[key(mc)].eventID = data_[key(mc.scheduleID())].eventID;
    data_[key(mc)].moduleStart = now();
  }

  void
  TimeTracker::recordTime(ModuleContext const& mc, string const& suffix)
  {
    auto const& d = data_[key(mc)];
    auto const t = chrono::duration<double>{now() - d.moduleStart}.count();
    timeModuleTable_.insert(d.eventID.run(),
                            d.eventID.subRun(),
                            d.eventID.event(),
                            mc.pathName(),
                            mc.moduleLabel(),
                            mc.moduleName() + suffix,
                            t);
  }

  void
  TimeTracker::logToDestination_(Statistics const& evt,
                                 vector<Statistics> const& modules)
  {
    size_t width{30};
    auto identifier_size = [](Statistics const& s) {
      return s.path.size() + s.mod_label.size() + s.mod_type.size() +
             2; // Don't forget the two ':'s.
    };
    cet::for_all(modules, [&identifier_size, &width](auto const& mod) {
      width = max(width, identifier_size(mod));
    });
    ostringstream msgOss;
    HorizontalRule const rule{width + 4 + 5 * 14 + 12};
    msgOss << '\n'
           << rule('=') << '\n'
           << std::setw(width + 2) << std::left << "TimeTracker printout (sec)"
           << boost::format(" %=12s ") % "Min"
           << boost::format(" %=12s ") % "Avg"
           << boost::format(" %=12s ") % "Max"
           << boost::format(" %=12s ") % "Median"
           << boost::format(" %=12s ") % "RMS"
           << boost::format(" %=10s ") % "nEvts"
           << "\n";
    msgOss << rule('=') << '\n';
    if (evt.n == 0u) {
      msgOss << "[ No processed events ]\n";
    } else {
      // N.B. setw(width) applies to the first field in
      //      ostream& operator<<(ostream&, Statistics const&).
      msgOss << setw(width) << evt << '\n' << rule('-') << '\n';
      for (auto const& mod : modules) {
        msgOss << setw(width) << mod << '\n';
      }
    }
    msgOss << rule('=');
    mf::LogAbsolute("TimeTracker") << msgOss.str();
  }

} // namespace art

DECLARE_ART_SERVICE(art::TimeTracker, LEGACY)
DEFINE_ART_SERVICE(art::TimeTracker)
