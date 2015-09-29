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

art::
TimeTracker::TimeTracker(ServiceTable<Config> const & config, ActivityRegistry& iRegistry)
  : printSummary_( config().printSummary() )
  , dbMgr_            ( config().dbOutput().filename() )
  , overwriteContents_( config().dbOutput().overwrite() )
    // table headers
  , timeReportTuple_( {"ReportType","Min","Mean","Max","Median","RMS","nEvts" } )
  , timeEventTuple_ ( {"Run","Subrun","Event","Time" } )
  , timeModuleTuple_( {"Run","Subrun","Event","PathModuleId","Time"} )
    // tables
  , timeReportTable_( dbMgr_.get(), "TimeReport", timeReportTuple_, true ) // always recompute reports
  , timeEventTable_ ( dbMgr_.get(), "TimeEvent" , timeEventTuple_ , overwriteContents_ )
  , timeModuleTable_( dbMgr_.get(), "TimeModule", timeModuleTuple_, overwriteContents_ )

{
  iRegistry.sPreProcessPath.watch(this, &TimeTracker::prePathProcessing);

  iRegistry.sPostEndJob.watch(this, &TimeTracker::postEndJob);

  iRegistry.sPreProcessEvent.watch(this, &TimeTracker::preEventProcessing);
  iRegistry.sPostProcessEvent.watch(this, &TimeTracker::postEventProcessing);

  iRegistry.sPreModule.watch(this, &TimeTracker::preModule);
  iRegistry.sPostModule.watch(this, &TimeTracker::postModule);
}

//======================================================================
void art::TimeTracker::prePathProcessing(std::string const& pathname)
{
  pathname_ = pathname;
}

//======================================================================
void art::TimeTracker::postEndJob()
{

  timeEventTable_ .flush();

  double const evtTime_min    = sqlite::min   ( dbMgr_.get(), "TimeEvent", "Time" );
  double const evtTime_max    = sqlite::max   ( dbMgr_.get(), "TimeEvent", "Time" );
  double const evtTime_mean   = sqlite::mean  ( dbMgr_.get(), "TimeEvent", "Time" );
  double const evtTime_median = sqlite::median( dbMgr_.get(), "TimeEvent", "Time" );
  double const evtTime_rms    = sqlite::rms   ( dbMgr_.get(), "TimeEvent", "Time" );

  if ( dbMgr_.logToDb() ) {
    timeReportTable_.insert( "Full event",
                             evtTime_min,
                             evtTime_mean,
                             evtTime_max,
                             evtTime_median,
                             evtTime_rms,
                             timeEventTable_.lastRowid() );
  }

  // This done here (instead of later) to determine the formatting
  // width in the printout
  timeModuleTable_.flush();
  const std::vector<std::string> modules
    = sqlite::getUniqueEntries<std::string>( dbMgr_.get(), "TimeModule", "PathModuleId" );

  std::size_t width(30);
  std::for_each( modules.begin(),
                 modules.end(),
                 [&width](const auto& id) { width = std::max( width, id.size() ); } );

  std::ostringstream msgOss;

  if ( printSummary_ ) {
    msgOss << std::string(width+4+5*14+12,'=') << "\n";
    msgOss << std::setw(width+2) << std::left << "TimeTracker printout (sec)"
           << boost::format(" %=12s ") % "Min"
           << boost::format(" %=12s ") % "Avg"
           << boost::format(" %=12s ") % "Max"
           << boost::format(" %=12s ") % "Median"
           << boost::format(" %=12s ") % "RMS"
           << boost::format(" %=10s ") % "nEvts" << "\n";

    msgOss << std::string(width+4+5*14+12,'=') << "\n";

    msgOss << setw(width) << "Full event" << "  "
           << boost::format(" %=12g ") % evtTime_min
           << boost::format(" %=12g ") % evtTime_mean
           << boost::format(" %=12g ") % evtTime_max
           << boost::format(" %=12g ") % evtTime_median
           << boost::format(" %=12g ") % evtTime_rms
           << boost::format(" %=10d ") % timeEventTable_.lastRowid() << "\n";

    msgOss << std::string(width+4+5*14+12,'-') << "\n";
  }


  for ( auto const & mod : modules ) {

    std::string const ddl =
      "CREATE TABLE temp.tmpModTable AS "s +
      "SELECT * FROM TimeModule WHERE PathModuleId='"s+mod+"'"s;

    sqlite::exec( dbMgr_.get(), ddl );

    double const modTime_min    = sqlite::min   ( dbMgr_.get(), "temp.tmpModTable", "Time" );
    double const modTime_max    = sqlite::max   ( dbMgr_.get(), "temp.tmpModTable", "Time" );
    double const modTime_mean   = sqlite::mean  ( dbMgr_.get(), "temp.tmpModTable", "Time" );
    double const modTime_median = sqlite::median( dbMgr_.get(), "temp.tmpModTable", "Time" );
    double const modTime_rms    = sqlite::rms   ( dbMgr_.get(), "temp.tmpModTable", "Time" );

    uint32_t const nEvts = sqlite::query_db<uint32_t>( dbMgr_.get(), "select count(*) from temp.tmpModTable" );

    if ( printSummary_ ) {
      msgOss << setw(width) << mod << "  "
             << boost::format(" %=12g ") % modTime_min
             << boost::format(" %=12g ") % modTime_mean
             << boost::format(" %=12g ") % modTime_max
             << boost::format(" %=12g ") % modTime_median
             << boost::format(" %=12g ") % modTime_rms
             << boost::format(" %=10d ") % nEvts << "\n";
    }

    if ( dbMgr_.logToDb() ) {
      timeReportTable_.insert( mod,
                               modTime_min,
                               modTime_mean,
                               modTime_max,
                               modTime_median,
                               modTime_rms,
                               nEvts );
    }

    sqlite::dropTable( dbMgr_.get(), "temp.tmpModTable" );

  }

  if ( printSummary_ ) {
    msgOss << std::string(width+4+5*14+12,'=') << "\n";
    mf::LogAbsolute("TimeTracker") << msgOss.str();
  }

}

//======================================================================
void art::TimeTracker::preEventProcessing(Event const& ev)
{
  eventId_    = ev.id();
  eventStart_ = now();
}

void art::TimeTracker::postEventProcessing(Event const&)
{

  double const t = (now()-eventStart_).seconds();

  timeEventTable_.insert( eventId_.run(),
                          eventId_.subRun(),
                          eventId_.event(),
                          t );

}

//======================================================================
void art::TimeTracker::preModule(ModuleDescription const&)
{
  moduleStart_ = now();
}

void art::TimeTracker::postModule(ModuleDescription const& desc)
{
  double const t = (now()-moduleStart_).seconds();

  timeModuleTable_.insert( eventId_.run(),
                           eventId_.subRun(),
                           eventId_.event(),
                           pathname_+":"s+desc.moduleLabel()+":"s+desc.moduleName(),
                           t );

}

// ======================================================================

// The DECLARE macro call should be moved to the header file, should you
// create one.
DECLARE_ART_SERVICE(art::TimeTracker, LEGACY)
DEFINE_ART_SERVICE(art::TimeTracker)

// ======================================================================
