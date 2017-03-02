// ======================================================================
//
// MemoryTracker
//
// ======================================================================

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Optional/detail/LinuxMallInfo.h"
#include "art/Framework/Services/Optional/detail/LinuxProcData.h"
#include "art/Framework/Services/Optional/detail/LinuxProcMgr.h"
#include "art/Framework/Services/Optional/detail/MemoryTrackerLinuxCallbackPair.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "boost/format.hpp"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/Ntuple/Ntuple.h"
#include "cetlib/Ntuple/sqlite_DBmanager.h"
#include "cetlib/Ntuple/sqlite_helpers.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exempt_ptr.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/OptionalAtom.h"
#include "fhiclcpp/types/Sequence.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <bitset>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iterator>
#include <memory>
#include <sstream>
#include <tuple>

namespace art {

  class MemoryTracker {
  public:

    struct Config {
      using Name = fhicl::Name;
      using Comment = fhicl::Comment;
      fhicl::Sequence<std::string> printSummaries { Name("printSummaries"), {"general", "event", "module"} };
      fhicl::Atom<int> maxTableLength { Name("maxTableLength"), Comment("The 'maxTableLength' parameter bounds the maximum number of rows\n"
                                                                        "shown in the summary tables. Specifying a negative value (e.g. '-1')\n"
                                                                        "means that all rows should be shown."), 5};
      struct DBoutput {
        fhicl::Atom<std::string> filename { Name("filename"), "" };
        fhicl::Atom<bool> overwrite { Name("overwrite"), false };
      };
      fhicl::Table<DBoutput> dbOutput { Name("dbOutput") };
      fhicl::Atom<bool> includeMallocInfo { Name("includeMallocInfo"), false };
      fhicl::OptionalAtom<unsigned> ignoreTotal { Name("ignoreTotal"), Comment("The following parameter is deprecated and no longer used.")};
    };

    using Parameters = ServiceTable<Config>;
    MemoryTracker(ServiceTable<Config> const&, ActivityRegistry&);

  private:

    enum summary_type {GENERAL, EVENT, MODULE, ntypes};

    // Callbacks
    // ... Path level
    void prePathProcessing(std::string const&);

    // ... Event level
    void preEventProcessing (Event const&);
    void postEventProcessing(Event const&);

    // ... Module level
    void setCurrentData(ModuleDescription const&);
    void recordData(ModuleDescription const& md, std::string const& suffix);

    // ... Wrap up
    void postEndJob();

    std::bitset<ntypes> setbits_(std::vector<std::string> const&);
    bool checkMallocConfig_(std::string const&, bool);

    void recordPeakUsages_();
    void generalSummary_(std::ostringstream&);
    void eventSummary_(std::ostringstream&, std::string const& col, std::string const& header);
    void moduleSummary_(std::ostringstream&, std::string const& col, std::string const& header);

    detail::LinuxProcMgr procInfo_;

    // Options
    std::bitset<ntypes> printSummary_;
    int maxTableLength_;
    sqlite::DBmanager dbMgr_;
    bool overwriteContents_;
    bool includeMallocInfo_;

    std::string pathname_ {};
    detail::LinuxProcData::proc_array evtData_ {{0.}};
    art::EventID eventId_ {};
    std::size_t evtCount_ {};

    detail::LinuxProcData::proc_array modData_ {{0.}};

    template <unsigned N>
    using name_array = sqlite::name_array<N>;

    name_array<3u> peakUsageColumns_ {{"Name", "Value", "Description"}};
    name_array<5u> otherInfoColumns_ {{"ProcessStep", "ModuleLabel", "ModuleType", "DeltaVsize", "DeltaRSS"}};
    name_array<7u> eventColumns_     {{"Run", "SubRun", "Event", "Vsize", "DeltaVsize", "RSS", "DeltaRSS"}};
    name_array<10u> moduleColumns_   {{"Run", "SubRun", "Event", "Path", "ModuleLabel", "ModuleType", "Vsize", "DeltaVsize", "RSS", "DeltaRSS"}};
    name_array<8u> eventHeapColumns_ {{"EvtRowId", "arena", "ordblks", "keepcost", "hblkhd", "hblks", "uordblks", "fordblks"}};
    name_array<8u> moduleHeapColumns_{{"ModRowId", "arena", "ordblks", "keepcost", "hblkhd", "hblks", "uordblks", "fordblks"}};

    using CallbackPair = detail::CallbackPair;
    using peakUsage_t = ntuple::Ntuple<std::string,double,std::string>;
    using otherInfo_t = CallbackPair::otherInfo_t;
    using memEvent_t = ntuple::Ntuple<uint32_t,uint32_t,uint32_t,double,double,double,double>;
    using memModule_t = ntuple::Ntuple<uint32_t,uint32_t,uint32_t,std::string,std::string,std::string,double,double,double,double>;
    using memHeap_t = ntuple::Ntuple<sqlite_int64,int,int,int,int,int,int,int>;

    peakUsage_t peakUsageTable_;
    otherInfo_t otherInfoTable_;
    memEvent_t eventTable_;
    memModule_t moduleTable_;
    std::unique_ptr<memHeap_t> eventHeapTable_;
    std::unique_ptr<memHeap_t> moduleHeapTable_;

    CallbackPair modConstruction_;
    CallbackPair modBeginJob_;
    CallbackPair modEndJob_;
    CallbackPair modBeginRun_;
    CallbackPair modEndRun_;
    CallbackPair modBeginSubRun_;
    CallbackPair modEndSubRun_;

  }; // MemoryTracker

}  // art

//====================================================
// Implementation below

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

  auto convertToEvtIdData(sqlite::stringstream& entry)
  {
    art::RunNumber_t run;
    art::SubRunNumber_t srun;
    art::EventNumber_t evt;
    double v;
    double dv;
    double rss;
    double drss;

    entry >> run >> srun >> evt;
    std::ostringstream id;
    id << art::EventID{run,srun,evt};

    entry >> v >> dv >> rss >> drss;

    if (!entry.empty()) {
      throw art::Exception{art::errors::LogicError,"Extra fields in sqlite query result not used."};
    }

    return std::make_pair(id.str(), memdata::MemData{v, dv, rss, drss});
  }

  //========================================================================
  namespace aliases {
    using eventData_t     = decltype(convertToEvtIdData(std::declval<sqlite::stringstream&>()));
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
                                  ActivityRegistry& iReg)
  : printSummary_  {setbits_(config().printSummaries())}
  , maxTableLength_{config().maxTableLength()}
  , dbMgr_         {config().dbOutput().filename()}
  , overwriteContents_{config().dbOutput().overwrite()}
  , includeMallocInfo_{checkMallocConfig_(config().dbOutput().filename(),
                                          config().includeMallocInfo())}
    // tables
  , peakUsageTable_ {dbMgr_, "PeakUsage", peakUsageColumns_, true} // always recompute the peak usage
  , otherInfoTable_ {dbMgr_, "OtherInfo", otherInfoColumns_, overwriteContents_}
  , eventTable_     {dbMgr_, "EventInfo", eventColumns_, overwriteContents_}
  , moduleTable_    {dbMgr_, "ModuleInfo", moduleColumns_, overwriteContents_}
  , eventHeapTable_ {includeMallocInfo_ ? std::make_unique<memHeap_t>(dbMgr_, "EventMallocInfo" , eventHeapColumns_ ) : nullptr}
  , moduleHeapTable_{includeMallocInfo_ ? std::make_unique<memHeap_t>(dbMgr_, "ModuleMallocInfo", moduleHeapColumns_) : nullptr}
    // instantiate the class templates
  , modConstruction_{otherInfoTable_, procInfo_, "Module Construction"}
  , modBeginJob_    {otherInfoTable_, procInfo_, "Module beginJob"}
  , modEndJob_      {otherInfoTable_, procInfo_, "Module endJob"}
  , modBeginRun_    {otherInfoTable_, procInfo_, "Module beginRun"}
  , modEndRun_      {otherInfoTable_, procInfo_, "Module endRun"}
  , modBeginSubRun_ {otherInfoTable_, procInfo_, "Module beginSubRun"}
  , modEndSubRun_   {otherInfoTable_, procInfo_, "Module endSubRun"}
{
  unsigned i{};
  if (config().ignoreTotal(i)) {
    mf::LogWarning("MemoryTracker") << "The 'services.MemoryTracker.ignoreTotal' parameter is deprecated;\n"
                                    << "its value of " << i << " will not be used.  Contact artists@fnal.gov for questions.";
  }
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
  iReg.sPreModule             .watch(  this                  , &MemoryTracker::setCurrentData);
  iReg.sPostModule            .watch([this](auto const& md){ this->recordData(md, ""s); });
  iReg.sPreWriteEvent         .watch(  this                  , &MemoryTracker::setCurrentData);
  iReg.sPostWriteEvent        .watch([this](auto const& md){ this->recordData(md, "(write)"s); });
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
                                          data[LinuxProcData::VSIZE],
                                          deltas[LinuxProcData::VSIZE],
                                          data[LinuxProcData::RSS],
                                          deltas[LinuxProcData::RSS]);

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
art::MemoryTracker::setCurrentData(ModuleDescription const&)
{
  modData_ = procInfo_.getCurrentData();
}

void
art::MemoryTracker::recordData(ModuleDescription const& md, std::string const& suffix)
{
  auto const data = procInfo_.getCurrentData();
  auto const deltas = data-modData_;

  moduleTable_.insert(eventId_.run(),
                                           eventId_.subRun(),
                                           eventId_.event(),
                                           pathname_,
                                           md.moduleLabel(),
                                           md.moduleName()+suffix,
                                           data[LinuxProcData::VSIZE],
                                           deltas[LinuxProcData::VSIZE],
                                           data[LinuxProcData::RSS],
                                           deltas[LinuxProcData::RSS]);

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
  recordPeakUsages_();

  if (printSummary_.none()) return;

  std::string const rule(100,'=');
  std::ostringstream msgOss;
  if (printSummary_.test(GENERAL)) {
    msgOss << rule << "\n\n";
    msgOss << std::left << "MemoryTracker General SUMMARY (all numbers in units of Mbytes)\n\n";
    generalSummary_(msgOss);
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
      throw Exception{errors::Configuration}
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
  peakUsageTable_.insert("VmPeak", procInfo_.getVmPeak(), "Peak virtual memory (MB)");
  peakUsageTable_.insert("VmHWM", procInfo_.getVmHWM(), "Peak resident set size (MB)");
}

//======================================================================
void
art::MemoryTracker::generalSummary_(std::ostringstream& oss)
{
  peakUsageTable_.flush();
  using namespace sqlite;
  result rVMax;
  result rRMax;
  rVMax << select("Value").from(peakUsageTable_).where("Name='VmPeak'");
  rRMax << select("Value").from(peakUsageTable_).where("Name='VmHWM'");
  double vmMax;
  double rssMax;
  throw_if_empty(rVMax) >> vmMax;
  throw_if_empty(rRMax) >> rssMax;

  oss << "  Peak virtual memory usage (VmPeak)  : " << vmMax << " Mbytes\n"
      << "  Peak resident set size usage (VmHWM): " << rssMax << " Mbytes\n"
      << "\n\n";

  otherInfoTable_.flush();

  result rSteps;
  result rMods;
  rSteps << select_distinct("ProcessStep").from(otherInfoTable_);
  rMods << select_distinct("ModuleLabel","ModuleType").from(otherInfoTable_);

  std::vector<std::string> steps;
  rSteps >> steps;

  std::vector<std::string> mods;
  for (auto& r : rMods) {
    std::string label{}, type{};
    r >> label >> type;
    mods.emplace_back(label+":"s+type);
  }

  // Calculate column widths
  std::size_t sWidth{}, mWidth{};
  cet::for_all(steps, [&sWidth](auto const& s){ sWidth = std::max(sWidth, s.size()); });
  cet::for_all(mods , [&mWidth](auto const& s){ mWidth = std::max(mWidth, s.size()); });

  std::string const rule = std::string(sWidth+2+mWidth+2+2*12,'=');

  oss << setw(sWidth+2) << "ProcessStep"
      << setw(mWidth+2) << "Module ID"
      << boost::format(" %_=10s ") % "\u0394 Vsize"
      << boost::format(" %_=10s ") % "\u0394 RSS"
      << "\n" << rule << "\n";

  std::string cachedStep {};
  std::size_t i {};

  result r;
  r << select("*").from(otherInfoTable_);
  for (auto& entry : r) {
    std::string step;
    std::string modLabel;
    std::string modType;
    double dv;
    double drss;

    entry >> step >> modLabel >> modType >> dv >> drss;

    if (cachedStep != step) {
      cachedStep = step;
      if (i != 0ull) {
        oss << std::string(sWidth+2+mWidth+2+2*12,'-') << "\n";
      }
    }

    oss << setw(sWidth+2) << std::left << step
        << setw(mWidth+2) << std::left << modLabel+":"s+modType
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
  using namespace sqlite;

  result r;
  r << select("*").from(eventTable_).where(column+" > 0").order_by(column,"desc").limit(maxTableLength_);

  for (auto& entry : r) {
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
    oss << "  [ no events ]\n";
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

  using namespace sqlite;
  result r;
  r << select_distinct("Path","ModuleLabel","ModuleType").from(moduleTable_);

  // Fill map, which will have form
  //
  //   mod1 : event1_data [ std::vector<std::string> ]
  //          event2_data [ "" ]
  //          event3_data [ "" ]
  //   mod2 : event1_data [ std::vector<std::string> ], etc.
  //

  orderedMap_t<modName_t,eventDataList_t> modMap;

  for (auto& row : r) {
    std::string path {};
    std::string mod_label {};
    std::string mod_type {};
    row >> path >> mod_label >> mod_type;
    std::string const ddl =
      "CREATE TABLE temp.tmpModTable AS "s +
      "SELECT * FROM ModuleInfo WHERE Path='"s+path+"'"s +
      " AND ModuleLabel='"s+mod_label+"'"s +
      " AND ModuleType='"s+mod_type+"'"s;
    sqlite::exec(dbMgr_, ddl);

    std::string const& columns = "Run,Subrun,Event,Vsize,DeltaVsize,RSS,DeltaRSS";

    eventDataList_t evtList;
    for (auto& entry : query(dbMgr_,
                             "SELECT "s+columns+" FROM temp.tmpModTable "s+
                             "WHERE "+column+" > 0 ORDER BY "s+column+" DESC LIMIT "s+std::to_string(maxTableLength_))) {
      auto evtData = convertToEvtIdData(entry);
      evtList.push_back(std::move(evtData));
    }
    modMap.emplace_back(path+":"s+mod_label+":"s+mod_type, std::move(evtList));

    sqlite::dropTable(dbMgr_, "temp.tmpModTable");
  }

  // Calculate widths
  std::size_t modWidth{}, evtWidth{};
  for (auto const& mod : modMap) {
    modWidth = std::max(modWidth, mod.first.size());
    for (auto const& evtInfo : mod.second) {
      evtWidth = std::max(evtWidth, std::get<std::string>(evtInfo).size());
    }
  }

  std::size_t const width = std::max({header.size(), modWidth, evtWidth});
  std::string const rule(width+4+4*12,'=');

  oss << banners::vsize_rss(header, width+2, 10) << "\n"
      << rule  << "\n";

  for (auto itMod = modMap.cbegin(); itMod != modMap.cend() ; ++itMod) {
    oss << itMod->first << "\n";

    if (itMod->second.empty()) {
      oss << "  [ no events ]\n";
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

DECLARE_ART_SERVICE(art::MemoryTracker, LEGACY)
DEFINE_ART_SERVICE(art::MemoryTracker)
