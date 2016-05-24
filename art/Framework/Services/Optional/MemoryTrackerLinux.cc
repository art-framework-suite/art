// ======================================================================
//
// MemoryTracker
//
// ======================================================================

#include "art/Framework/Services/Optional/MemoryTracker.h"
#include "art/Framework/Services/Optional/detail/LinuxMallInfo.h"
#include "art/Ntuple/Ntuple.h"
#include "art/Ntuple/sqlite_helpers.h"
#include "boost/format.hpp"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iterator>
#include <sstream>

using namespace std::string_literals;
using namespace ntuple;
using std::setw;

namespace {

  //====================================================================
  // Helpers

  //====================================================================
  namespace ostream_sanitize {

    // Turning '0.000' entries to '0'
    template <unsigned WIDTH = 10>
    struct sanitizeZero {
      sanitizeZero(double const v) : val{v} , width{WIDTH} {}
      double val;
      std::size_t width;
    };

    template <typename T, unsigned U = 10>
    decltype(auto) operator<<(T&& t, sanitizeZero<U>&& sz)
    {
      std::string const width = std::to_string(sz.width);
      t << (( sz.val == 0. ) ? boost::format(" %="s+width+"d "s ) % 0 : boost::format(" %="s+width+".3f "s) % sz.val );
      return std::forward<T>(t);
    }

  } // (anon::)ostream_sanitize

  //====================================================================
  namespace banners {

    std::string vsize_rss(std::string const& header,
                          std::size_t const firstColWidth,
                          std::size_t const latterColWidths)
    {
      std::string const width = std::to_string(latterColWidths);
      std::ostringstream oss;
      oss << setw(firstColWidth) << std::left << header
          << boost::format(" %="s+width+"s "s) % "Vsize"
          << boost::format(" %_="s+width+"s "s) % "\u0394 Vsize"
          << boost::format(" %="s+width+"s "s) % "RSS"
          << boost::format(" %_="s+width+"s "s) % "\u0394 RSS";
      return oss.str();
    }

  } // (anon::)banners

  //====================================================================
  namespace memdata {

    struct MemData {
      MemData(double const v, double const dv, double const r, double const dr)
        : vsize{v}, deltaVsize{dv}, rss{r}, deltaRss{dr}
      {}

      double vsize;
      double deltaVsize;
      double rss;
      double deltaRss;
    };

    template< typename T>
    decltype(auto) operator<<(T&& t, MemData const& data)
    {
      t << ostream_sanitize::sanitizeZero<>(data.vsize)
        << ostream_sanitize::sanitizeZero<>(data.deltaVsize)
        << ostream_sanitize::sanitizeZero<>(data.rss)
        << ostream_sanitize::sanitizeZero<>(data.deltaRss);
      return std::forward<T>(t);
    }
  } // (anon::)memdata


  //========================================================================
  // Other helpers

  enum sql_constants { ROWID };
  auto convertToEvtIdData(sqlite::stringstream& entry)
  {
    std::size_t rowid;
    art::RunNumber_t run;
    art::SubRunNumber_t srun;
    art::EventNumber_t evt;
    double v;
    double dv;
    double rss;
    double drss;

    entry >> rowid >> run >> srun >> evt;
    std::ostringstream id;
    id << art::EventID{run,srun,evt};

    entry >> v >> dv >> rss >> drss;

    if (!entry.empty()) {
      throw art::Exception{art::errors::LogicError,"Extra fields in sqlite query result not used."};
    }

    return std::make_tuple(rowid, id.str(), memdata::MemData{v, dv, rss, drss});
  }

  //========================================================================
  namespace aliases {
    using eventData_t     = std::tuple<std::size_t,std::string,memdata::MemData>;
    using eventDataList_t = std::vector<eventData_t>;
    using modName_t       = std::string;
    template<typename KEY, typename VALUE> using orderedMap_t = std::vector<std::pair<KEY,VALUE>>;
  }

} // anon. namespace

//======================================================================================
using namespace art::detail;
using namespace ostream_sanitize;
using namespace banners;
using namespace memdata;
using namespace aliases;

art::MemoryTracker::MemoryTracker(ServiceTable<Config> const& config,
                                  ActivityRegistry & iReg)
  : numToSkip_     {config().ignoreTotal()}
  , printSummary_  {setbits_( config().printSummaries() )}
  , dbMgr_         {config().dbOutput().filename()}
  , overwriteContents_{config().dbOutput().overwrite()}
  , includeMallocInfo_{checkMallocConfig_(config().dbOutput().filename(),
                                          config().includeMallocInfo())}
    // column headings
  , summaryTuple_   {"ProcessStep", "ModuleId", "DeltaVsize", "DeltaRSS"}
  , eventTuple_     {"Run", "Subrun", "Event", "Vsize", "DeltaVsize", "RSS", "DeltaRSS"}
  , moduleTuple_    {"Run", "Subrun", "Event", "PathModuleId", "Vsize", "DeltaVsize", "RSS", "DeltaRSS"}
  , eventHeapTuple_ {"EvtRowId","arena", "ordblks", "keepcost", "hblkhd", "hblks", "uordblks", "fordblks"}
  , moduleHeapTuple_{"ModRowId","arena", "ordblks", "keepcost", "hblkhd", "hblks", "uordblks", "fordblks"}
    // tables
  , summaryTable_   {dbMgr_, "Summary", summaryTuple_, true} // always recompute the summary
  , eventTable_     {dbMgr_, "EventInfo" , eventTuple_, overwriteContents_}
  , moduleTable_    {dbMgr_, "ModuleInfo", moduleTuple_, overwriteContents_}
  , eventHeapTable_ {includeMallocInfo_ ? std::make_unique<memHeap_t>(dbMgr_, "EventMallocInfo" , eventHeapTuple_ ) : nullptr}
  , moduleHeapTable_{includeMallocInfo_ ? std::make_unique<memHeap_t>(dbMgr_, "ModuleMallocInfo", moduleHeapTuple_) : nullptr}
    // instantiate the class templates
    //  , evtSource_      {summaryTable_, procInfo_, evtCount_, "Event source"}
  , modConstruction_{summaryTable_, procInfo_, evtCount_, "Module Construction"}
  , modBeginJob_    {summaryTable_, procInfo_, evtCount_, "Module beginJob"}
  , modEndJob_      {summaryTable_, procInfo_, evtCount_, "Module endJob"}
  , modBeginRun_    {summaryTable_, procInfo_, evtCount_, "Module beginRun"}
  , modEndRun_      {summaryTable_, procInfo_, evtCount_, "Module endRun"}
  , modBeginSubRun_ {summaryTable_, procInfo_, evtCount_, "Module beginSubRun"}
  , modEndSubRun_   {summaryTable_, procInfo_, evtCount_, "Module endSubRun"}
{
  iReg.sPreModuleConstruction .watch( &this->modConstruction_, &CallbackPair::pre  );
  iReg.sPostModuleConstruction.watch( &this->modConstruction_, &CallbackPair::post );
  iReg.sPreModuleBeginJob     .watch( &this->modBeginJob_    , &CallbackPair::pre  );
  iReg.sPostModuleBeginJob    .watch( &this->modBeginJob_    , &CallbackPair::post );
  iReg.sPreModuleBeginRun     .watch( &this->modBeginRun_    , &CallbackPair::pre  );
  iReg.sPostModuleBeginRun    .watch( &this->modBeginRun_    , &CallbackPair::post );
  iReg.sPreModuleBeginSubRun  .watch( &this->modBeginSubRun_ , &CallbackPair::pre  );
  iReg.sPostModuleBeginSubRun .watch( &this->modBeginSubRun_ , &CallbackPair::post );
  iReg.sPreProcessPath        .watch(  this                  , &MemoryTracker::prePathProcessing);
  iReg.sPreProcessEvent       .watch(  this                  , &MemoryTracker::preEventProcessing);
  iReg.sPostProcessEvent      .watch(  this                  , &MemoryTracker::postEventProcessing);
  iReg.sPreModule             .watch(  this                  , &MemoryTracker::preModule);
  iReg.sPostModule            .watch(  this                  , &MemoryTracker::postModule);
  iReg.sPreModuleEndSubRun    .watch( &this->modEndSubRun_   , &CallbackPair::pre  );
  iReg.sPostModuleEndSubRun   .watch( &this->modEndSubRun_   , &CallbackPair::post );
  iReg.sPreModuleEndRun       .watch( &this->modEndRun_      , &CallbackPair::pre  );
  iReg.sPostModuleEndRun      .watch( &this->modEndRun_      , &CallbackPair::post );
  iReg.sPreModuleEndJob       .watch( &this->modEndJob_      , &CallbackPair::pre  );
  iReg.sPostModuleEndJob      .watch( &this->modEndJob_      , &CallbackPair::post );
  iReg.sPostEndJob            .watch(  this                  , &MemoryTracker::postEndJob);
}

//======================================================================
void
art::MemoryTracker::prePathProcessing(std::string const& pathname)
{
  pathname_ = pathname;
}

//======================================================================
void
art::MemoryTracker::preEventProcessing(Event const& e)
{
  ++evtCount_;
  eventId_ = e.id();
  evtData_ = procInfo_.getCurrentData();
}

void
art::MemoryTracker::postEventProcessing(Event const&)
{
  auto const data = procInfo_.getCurrentData();
  auto const deltas = data-evtData_;

  eventTable_.insert(eventId_.run(),
                     eventId_.subRun(),
                     eventId_.event(),
                     data.at(LinuxProcData::VSIZE),
                     deltas.at(LinuxProcData::VSIZE),
                     data.at(LinuxProcData::RSS),
                     deltas.at(LinuxProcData::RSS));

  if (includeMallocInfo_) {
    auto minfo = LinuxMallInfo().get();
    eventHeapTable_->insert(eventTable_.lastRowid(),
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
art::MemoryTracker::preModule(ModuleDescription const&)
{
  modData_ = procInfo_.getCurrentData();
}

void
art::MemoryTracker::postModule(ModuleDescription const& md)
{
  auto const data = procInfo_.getCurrentData();
  auto const deltas = data-modData_;

  moduleTable_.insert(eventId_.run(),
                      eventId_.subRun(),
                      eventId_.event(),
                      pathname_+":"s+md.moduleLabel()+":"s+md.moduleName(),
                      data.at(LinuxProcData::VSIZE),
                      deltas.at(LinuxProcData::VSIZE),
                      data.at(LinuxProcData::RSS),
                      deltas.at(LinuxProcData::RSS));

  if (includeMallocInfo_) {
    auto minfo = LinuxMallInfo().get();
    moduleHeapTable_->insert(moduleTable_.lastRowid(),
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
  if (printSummary_.none()) return;

  std::string const rule(100,'=');
  std::ostringstream msgOss;
  if (printSummary_.test(GENERAL)) {
    msgOss << rule << "\n\n";
    msgOss << std::left << "MemoryTracker General SUMMARY (all numbers in units of Mbytes)\n\n";
    generalSummary_( msgOss );
  }
  if (printSummary_.test(EVENT)) {
    msgOss << rule << "\n\n";
    msgOss << std::left << "MemoryTracker Per-event SUMMARY\n\n";
    eventSummary_(msgOss, "DeltaVsize", "Events increasing Vsize (Mbytes)");
    eventSummary_(msgOss, "Vsize"     , "Events with large Vsize (Mbytes)");
  }
  if (printSummary_.test(MODULE)) {
    msgOss << rule << "\n\n";
    msgOss << std::left << "MemoryTracker Per-module SUMMARY\n\n";
    moduleSummary_(msgOss, "DeltaVsize", "Modules increasing Vsize (Mbytes)");
    moduleSummary_(msgOss, "Vsize"     , "Modules with large Vsize (Mbytes)");
  }

  msgOss << rule << "\n";

  mf::LogAbsolute("MemoryTracker") << msgOss.str();
}

//======================================================================
// Private member functions

std::bitset<art::MemoryTracker::ntypes>
art::MemoryTracker::setbits_(std::vector<std::string> const& pset)
{
  std::bitset<ntypes> bset;

  if (pset.empty())
    return bset;

  std::set<std::string> const pset_bits (pset.cbegin(), pset.cend());

  if (pset_bits.find("*") != pset_bits.cend()) {
    if (pset_bits.size() == 1ull) bset.set();
    else {
      throw art::Exception{art::errors::Configuration}
      << "The summary option '*' cannot be included with any other options.";
    }
  }

  if (pset_bits.find("general") != pset_bits.cend()) bset.set(GENERAL);
  if (pset_bits.find("event"  ) != pset_bits.cend()) bset.set(EVENT  );
  if (pset_bits.find("module" ) != pset_bits.cend()) bset.set(MODULE );

  return bset;
}

//======================================================================
bool
art::MemoryTracker::checkMallocConfig_(std::string const& dbfilename,
                                       bool const include)
{
  if (include && dbfilename.empty()) {
    std::string const errmsg =
      "\n'includeMallocInfo : true' is valid only if a nonempty db filename is specified:\n\n"s+
      "   MemoryTracker: {\n"
      "      includeMallocInfo: true\n"
      "      db: {\n"
      "         filename: \"your_filename.db\"\n"
      "      }\n"
      "   }\n\n";
    throw art::Exception{art::errors::Configuration} << errmsg;
  }
  return include;
}

//======================================================================
void
art::MemoryTracker::generalSummary_(std::ostringstream& oss)
{
  summaryTable_.flush();

  auto const& steps = sqlite::getUniqueEntries<std::string>(dbMgr_, "Summary", "ProcessStep");
  auto const& mods  = sqlite::getUniqueEntries<std::string>(dbMgr_, "Summary", "ModuleId");

  // Calculate column widths
  std::size_t sWidth{}, mWidth{};
  cet::for_all(steps, [&sWidth](auto const& s){ sWidth = std::max(sWidth, s.size()); });
  cet::for_all(mods , [&mWidth](auto const& s){ mWidth = std::max(mWidth, s.size()); });

  std::string const rule = std::string(sWidth+2+mWidth+2+2*12,'=');

  oss << "  Peak virtual memory usage (VmPeak): " << procInfo_.getVmPeak() << " Mbytes"
      << "\n\n"
      << setw(sWidth+2) << "ProcessStep"
      << setw(mWidth+2) << "Module ID"
      << boost::format(" %_=10s ") % "\u0394 Vsize"
      << boost::format(" %_=10s ") % "\u0394 RSS"
      << "\n" << rule << "\n";

  std::string cachedStep {};
  std::size_t i {};
  for (auto& entry : sqlite::query_db(dbMgr_, "SELECT * FROM Summary")) {
    std::string step;
    std::string module;
    double dv;
    double drss;

    entry >> step >> module >> dv >> drss;

    if (cachedStep != step) {
      cachedStep = step;
      if (i != 0ull) {
        oss << std::string(sWidth+2+mWidth+2+2*12,'-') << "\n";
      }
    }

    oss << setw(sWidth+2) << std::left << step
        << setw(mWidth+2) << std::left << module
        << sanitizeZero<>(dv)
        << sanitizeZero<>(drss)
        << "\n";
    ++i;

  }
  oss << "\n";
}

//======================================================================
void
art::MemoryTracker::eventSummary_(std::ostringstream& oss,
                                  std::string const& column,
                                  std::string const& header)
{
  eventTable_.flush();

  eventDataList_t evtList;
  std::size_t i{};
  for (auto& entry : sqlite::query_db(dbMgr_,
                                      "SELECT rowid,* FROM EventInfo "s+
                                      "WHERE "s+column+" > 0 ORDER BY "s+column+" DESC LIMIT 5"s,
                                      false)) {
    if (i++ < numToSkip_) continue;
    auto evtData = convertToEvtIdData(entry);
    evtList.push_back(std::move(evtData));
  }

  std::size_t evtWidth{};
  for (auto const& data : evtList) {
    evtWidth = std::max(evtWidth, std::get<std::string>(data).size());
  }

  std::size_t const width = std::max(header.size(), evtWidth);
  std::string const rule  = std::string(width+4+4*12,'=');

  oss << banners::vsize_rss(header, width+4, 10) << "\n"
      << rule << "\n";

  if (evtList.empty()) {
    oss << "  [[ no events ]] " << "\n";
  }

  for (auto const& data : evtList) {
    std::ostringstream preamble;
    preamble << "  ";
    preamble << setw(evtWidth+2) << std::left << std::get<std::string>(data);
    oss << setw(width+4) << preamble.str() << std::get<MemData>(data) << "\n";
  }
  oss << "\n";
}

//======================================================================
void
art::MemoryTracker::moduleSummary_(std::ostringstream& oss,
                                   std::string const& column,
                                   std::string const& header)
{
  moduleTable_.flush();

  std::vector<std::string> const& mods = sqlite::getUniqueEntries<std::string>(dbMgr_, "ModuleInfo", "PathModuleId");

  // Fill map, which will have form
  //
  //   mod1 : event1_data [ std::vector<std::string> ]
  //          event2_data [ "" ]
  //          event3_data [ "" ]
  //   mod2 : event1_data [ std::vector<std::string> ], etc.
  //

  orderedMap_t<modName_t,eventDataList_t> modMap;

  for (auto const& mod : mods) {
    std::string const ddl =
      "CREATE TABLE temp.tmpModTable AS "s +
      "SELECT * FROM ModuleInfo WHERE PathModuleId='"s+mod+"'"s;
    sqlite::exec(dbMgr_, ddl);

    std::string const& columns = "rowid,Run,Subrun,Event,Vsize,DeltaVsize,RSS,DeltaRSS";

    eventDataList_t evtList;
    std::size_t i{};
    for (auto& entry : sqlite::query_db(dbMgr_,
                                        "SELECT "s+columns+" FROM temp.tmpModTable "s+
                                        "WHERE "+column+" > 0 ORDER BY "s+column+" DESC LIMIT 5"s,
                                        false)) {
      if (i++ < numToSkip_) continue;
      auto evtData = convertToEvtIdData(entry);
      evtList.push_back(std::move(evtData));
    }
    modMap.emplace_back(mod, std::move(evtList));

    sqlite::dropTable(dbMgr_, "temp.tmpModTable");
  }

  // Calculate widths
  std::size_t modWidth{0}, evtWidth{0};
  for (auto const& mod : modMap) {
    modWidth = std::max(modWidth, mod.first.size());
    for (auto const& evtInfo : mod.second) {
      evtWidth = std::max(evtWidth, std::get<std::string>(evtInfo).size());
    }
  }

  std::size_t const width = std::max({header.size(), modWidth, evtWidth});
  std::string const rule  = std::string(width+4+4*12,'=');

  oss << banners::vsize_rss(header, width+2, 10) << "\n"
      << rule  << "\n";

  for (auto itMod = modMap.cbegin(); itMod != modMap.cend() ; ++itMod) {
    oss << itMod->first << "\n";

    if (itMod->second.empty()) {
      oss << "  [[ no events ]] " << "\n";
    }

    for (auto const& evtInfo : itMod->second) {
      std::ostringstream preamble;
      preamble << "  ";
      preamble << setw(evtWidth+2) << std::left << std::get<std::string>(evtInfo);
      oss << setw(width+2) << preamble.str() << std::get<MemData>(evtInfo) << "\n";
    }

    if (std::next(itMod) != modMap.cend()) {
      oss << std::string(width+4+4*12,'-') << "\n";
    }

  }
  oss << "\n" ;

}

// Local variables:
// mode: c++
// End:
