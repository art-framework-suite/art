// ======================================================================
//
// MemoryTrackerLinux
//
// ======================================================================

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Optional/detail/constrained_multimap.h"
#include "art/Framework/Services/Optional/detail/LinuxMallInfo.h"
#include "art/Framework/Services/Optional/detail/LinuxProcData.h"
#include "art/Framework/Services/Optional/detail/LinuxProcMgr.h"
#include "art/Framework/Services/Optional/detail/MemoryTrackerLinuxCallbackPair.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Ntuple/Ntuple.h"
#include "art/Ntuple/sqlite_DBmanager.h"
#include "art/Ntuple/sqlite_helpers.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/MallocOpts.h"
#include "boost/format.hpp"
#include "cetlib/exempt_ptr.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

extern "C" {
#include <fcntl.h>
#include <unistd.h>
}

#include <bitset>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iterator>
#include <memory>
#include <sstream>
#include <tuple>
#include <type_traits>

using namespace std::string_literals;
using namespace ntuple;

namespace art {

#ifndef __linux__
  static_assert( 0, "MemoryTracker(Linux) incompatible with this operating system." );
#endif

  //======================================================================
  namespace detail {

    //====================================================================
    // helpers below encapsulated in 'inline' namespaces to separate
    // functionality.  All functions inside inline namespaces can be
    // accessed via 'detail::(inline namespace name optional::)func'.

    //====================================================================
    inline
    namespace ostream_sanitize {

      // Turning '0.000' entries to '0'
      template < unsigned WIDTH = 10 >
      struct sanitizeZero{
        sanitizeZero( double const v ) : val(v) , width(WIDTH) {}
        double val;
        std::size_t width;
      };

      template <typename T,unsigned U = 10>
      decltype(auto) operator<<( T&& t, sanitizeZero<U>&& sz ) {
        std::string width = std::to_string( sz.width );
        t << (( sz.val == 0. ) ? boost::format(" %="s+width+"d "s ) % 0 : boost::format(" %="s+width+".3f "s) % sz.val );
        return std::forward<T>( t );
      }

    } // detail::ostream_sanitize

    //====================================================================
    inline
    namespace banners {

      std::string vsize_rss( std::string const & header,
                             std::size_t const   firstColWidth,
                             std::size_t const   latterColWidths )
      {
        std::ostringstream oss;
        std::string const width = std::to_string( latterColWidths );
        oss << std::setw(firstColWidth) << std::left << header
            << boost::format(" %="s+width+"s "s) % "Vsize"
            << boost::format(" %_="s+width+"s "s) % "\u0394 Vsize"
            << boost::format(" %="s+width+"s "s) % "RSS"
            << boost::format(" %_="s+width+"s "s) % "\u0394 RSS";
        return oss.str();
      }

    } // detail::banners

    //====================================================================
    inline
    namespace memdata {

      struct MemData {
        MemData( double v, double dv, double r, double dr )
          : vsize(v), deltaVsize(dv), rss(r), deltaRss(dr)
        {}

        double vsize;
        double deltaVsize;
        double rss;
        double deltaRss;
      };

      template< typename T>
      decltype(auto) operator<<( T&& t, MemData const& data )
      {
        t << sanitizeZero<10>( data.vsize      )
          << sanitizeZero<10>( data.deltaVsize )
          << sanitizeZero<10>( data.rss        )
          << sanitizeZero<10>( data.deltaRss   );
        return std::forward<T>( t );
      }
    } // detail::memdata


    //========================================================================
    // Other helpers

    auto convertToEvtIdData( sqlite::stringstream & entry )
    {
      std::size_t        rowid{};
      art::RunNumber_t     run{};
      art::SubRunNumber_t srun{};
      art::EventNumber_t   evt{};
      double    v{};
      double   dv{};
      double  rss{};
      double drss{};

      entry >> rowid >> run >> srun >> evt;
      std::ostringstream id;
      id << art::EventID( run,srun,evt );

      entry >> v >> dv >> rss >> drss;

      if ( !entry.empty() ) {
        throw art::Exception(art::errors::LogicError,"Extra fields in sqlite query result not used.");
      }

      return std::make_tuple( rowid, id.str(), MemData( v,dv,rss,drss ) );
    }

    std::string const rule(100,'=');

  } // detail
} // art

//======================================================================================
using namespace art::detail;

namespace art {

  using eventData_t     = std::tuple<std::size_t,std::string,MemData>;
  using eventDataList_t = std::vector<eventData_t>;
  using modName_t       = std::string;
  template<typename KEY, typename VALUE> using orderedMap_t = std::vector<std::pair<KEY,VALUE>>;


  class MemoryTrackerLinux {
  public:

    MemoryTrackerLinux(fhicl::ParameterSet const &, ActivityRegistry &);

    // Path level
    void prePathProcessing(std::string const&);

    // Event level
    void preEventProcessing (Event const &);
    void postEventProcessing(Event const &);

    // Module level
    void preModule (ModuleDescription const &);
    void postModule(ModuleDescription const &);

    // Wrap up
    void postEndJob();

    enum summary_type { GENERAL, EVENT, MODULE, ntypes };

  private:

    std::bitset<ntypes> setbits_( std::vector<std::string> const & );
    bool checkMallocConfig_( std::string const &, bool );

    void generalSummary_( std::ostringstream& );
    void eventSummary_  ( std::ostringstream&, std::string const& col, std::string const& header );
    void moduleSummary_ ( std::ostringstream&, std::string const& col, std::string const& header );

    LinuxProcMgr procInfo_;

    // Options
    unsigned numToSkip_;
    std::bitset<ntypes> printSummary_;
    sqlite::DBmanager dbMgr_;
    bool     includeMallocInfo_;

    std::string pathname_;
    LinuxProcData::proc_array evtData_;
    art::EventID eventId_;
    std::size_t  evtCount_;

    LinuxProcData::proc_array modData_;

    name_array<4u> summaryTuple_;
    name_array<7u> eventTuple_;
    name_array<8u> moduleTuple_;
    name_array<8u> eventHeapTuple_;
    name_array<8u> moduleHeapTuple_;

    using memSummary_t = Ntuple<std::string,std::string,double,double>;
    using memEvent_t   = Ntuple<uint32_t,uint32_t,uint32_t,double,double,double,double>;
    using memModule_t  = Ntuple<uint32_t,uint32_t,uint32_t,std::string,double,double,double,double>;
    using memHeap_t    = Ntuple<sqlite_int64,int,int,int,int,int,int,int>;

    memSummary_t summaryTable_;
    memEvent_t   eventTable_;
    memModule_t  moduleTable_;
    std::unique_ptr<memHeap_t> eventHeapTable_;
    std::unique_ptr<memHeap_t> moduleHeapTable_;

    CallbackPair<SourceSummaryType> evtSource_;
    CallbackPair<ModuleSummaryType> modConstruction_;
    CallbackPair<ModuleSummaryType> modBeginJob_;
    CallbackPair<ModuleSummaryType> modEndJob_;
    CallbackPair<ModuleSummaryType> modBeginRun_;
    CallbackPair<ModuleSummaryType> modEndRun_;
    CallbackPair<ModuleSummaryType> modBeginSubRun_;
    CallbackPair<ModuleSummaryType> modEndSubRun_;

  }; // MemoryTrackerLinux

}  // art

// ======================================================================

namespace art {

  MemoryTrackerLinux::MemoryTrackerLinux(const fhicl::ParameterSet & pset,
                                         ActivityRegistry & iReg)
    : numToSkip_     (pset.get<unsigned>("ignoreTotal", 1))
    , printSummary_  (setbits_( pset.get<std::vector<std::string> >
                                ("printSummaries",
                                 std::vector<std::string>{"general","event","module"} )
                                )
                      )
    , dbMgr_ ( pset.get<std::string>("filename","" ) )
    , includeMallocInfo_( checkMallocConfig_(pset.get<std::string>("filename",""),
                                             pset.get<bool> ("includeMallocInfo", false) )
                          )
    , evtCount_()
      // column headings
    , summaryTuple_   ( { "ProcessStep", "ModuleId", "DeltaVsize", "DeltaRSS" } )
    , eventTuple_     ( { "Run", "Subrun", "Event", "Vsize", "DeltaVsize", "RSS", "DeltaRSS" } )
    , moduleTuple_    ( { "Run", "Subrun", "Event", "PathModuleId", "Vsize", "DeltaVsize", "RSS", "DeltaRSS" } )
    , eventHeapTuple_ ( { "EvtRowId","arena", "ordblks", "keepcost", "hblkhd", "hblks", "uordblks", "fordblks" } )
    , moduleHeapTuple_( { "ModRowId","arena", "ordblks", "keepcost", "hblkhd", "hblks", "uordblks", "fordblks" } )
      // tables
    , summaryTable_   ( dbMgr_.get(), "Summary", summaryTuple_ )
    , eventTable_     ( dbMgr_.get(), "EventInfo" , eventTuple_ )
    , moduleTable_    ( dbMgr_.get(), "ModuleInfo", moduleTuple_ )
    , eventHeapTable_ ( includeMallocInfo_ ? std::make_unique<memHeap_t>( dbMgr_.get(), "EventMallocInfo" , eventHeapTuple_  ) : nullptr )
    , moduleHeapTable_( includeMallocInfo_ ? std::make_unique<memHeap_t>( dbMgr_.get(), "ModuleMallocInfo", moduleHeapTuple_ ) : nullptr )
      // instantiate the class templates
    , evtSource_      ( summaryTable_, procInfo_, evtCount_, "Event source"        )
    , modConstruction_( summaryTable_, procInfo_, evtCount_, "Module Construction" )
    , modBeginJob_    ( summaryTable_, procInfo_, evtCount_, "Module beginJob"     )
    , modEndJob_      ( summaryTable_, procInfo_, evtCount_, "Module endJob"       )
    , modBeginRun_    ( summaryTable_, procInfo_, evtCount_, "Module beginRun"     )
    , modEndRun_      ( summaryTable_, procInfo_, evtCount_, "Module endRun"       )
    , modBeginSubRun_ ( summaryTable_, procInfo_, evtCount_, "Module beginSubRun"  )
    , modEndSubRun_   ( summaryTable_, procInfo_, evtCount_, "Module endSubRun"    )
  {
    using modDesc_cref = ModuleDescription const &;

    iReg.sPreModuleConstruction .watch( &this->modConstruction_, &CallbackPair<ModuleSummaryType>::pre <modDesc_cref> );
    iReg.sPostModuleConstruction.watch( &this->modConstruction_, &CallbackPair<ModuleSummaryType>::post<modDesc_cref> );
    iReg.sPreModuleBeginJob     .watch( &this->modBeginJob_    , &CallbackPair<ModuleSummaryType>::pre <modDesc_cref> );
    iReg.sPostModuleBeginJob    .watch( &this->modBeginJob_    , &CallbackPair<ModuleSummaryType>::post<modDesc_cref> );
    iReg.sPreModuleBeginRun     .watch( &this->modBeginRun_    , &CallbackPair<ModuleSummaryType>::pre <modDesc_cref> );
    iReg.sPostModuleBeginRun    .watch( &this->modBeginRun_    , &CallbackPair<ModuleSummaryType>::post<modDesc_cref> );
    iReg.sPreModuleBeginSubRun  .watch( &this->modBeginSubRun_ , &CallbackPair<ModuleSummaryType>::pre <modDesc_cref> );
    iReg.sPostModuleBeginSubRun .watch( &this->modBeginSubRun_ , &CallbackPair<ModuleSummaryType>::post<modDesc_cref> );
    iReg.sPreProcessPath        .watch(  this                  , &MemoryTrackerLinux::prePathProcessing  );
    iReg.sPreSource             .watch( &this->evtSource_      , &CallbackPair<SourceSummaryType>::pre  );
    iReg.sPostSource            .watch( &this->evtSource_      , &CallbackPair<SourceSummaryType>::post );
    iReg.sPreProcessEvent       .watch(  this                  , &MemoryTrackerLinux::preEventProcessing );
    iReg.sPostProcessEvent      .watch(  this                  , &MemoryTrackerLinux::postEventProcessing);
    iReg.sPreModule             .watch(  this                  , &MemoryTrackerLinux::preModule    );
    iReg.sPostModule            .watch(  this                  , &MemoryTrackerLinux::postModule   );
    iReg.sPreModuleEndSubRun    .watch( &this->modEndSubRun_   , &CallbackPair<ModuleSummaryType>::pre <modDesc_cref> );
    iReg.sPostModuleEndSubRun   .watch( &this->modEndSubRun_   , &CallbackPair<ModuleSummaryType>::post<modDesc_cref> );
    iReg.sPreModuleEndRun       .watch( &this->modEndRun_      , &CallbackPair<ModuleSummaryType>::pre <modDesc_cref> );
    iReg.sPostModuleEndRun      .watch( &this->modEndRun_      , &CallbackPair<ModuleSummaryType>::post<modDesc_cref> );
    iReg.sPreModuleEndJob       .watch( &this->modEndJob_      , &CallbackPair<ModuleSummaryType>::pre <modDesc_cref> );
    iReg.sPostModuleEndJob      .watch( &this->modEndJob_      , &CallbackPair<ModuleSummaryType>::post<modDesc_cref> );
    iReg.sPostEndJob            .watch(  this                  , &MemoryTrackerLinux::postEndJob   );

    typedef art::MallocOpts::opt_type opt_type;
    art::MallocOptionSetter & mopts = art::getGlobalOptionSetter();
    opt_type
      p_mmap_max = pset.get<int>("M_MMAP_MAX", -1),
      p_trim_thr = pset.get<int>("M_TRIM_THRESHOLD", -1),
      p_top_pad  = pset.get<int>("M_TOP_PAD", -1),
      p_mmap_thr = pset.get<int>("M_MMAP_THRESHOLD", -1);
    if (p_mmap_max >= 0) { mopts.set_mmap_max(p_mmap_max); }
    if (p_trim_thr >= 0) { mopts.set_trim_thr(p_trim_thr); }
    if (p_top_pad  >= 0) { mopts.set_top_pad(p_top_pad);   }
    if (p_mmap_thr >= 0) { mopts.set_mmap_thr(p_mmap_thr); }
    mopts.adjustMallocParams();
    if (mopts.hasErrors()) {
      mf::LogWarning("MemoryCheck")
        << "ERROR: Problem with setting malloc options\n"
        << mopts.error_message();
    }
    if (pset.get<bool>("dump", false) == true) {
      art::MallocOpts mo = mopts.get();
      mf::LogWarning("MemoryCheck") << "Malloc options: " << mo << "\n";
    }
  }

  //======================================================================
  void MemoryTrackerLinux::prePathProcessing(std::string const& pathname)
  {
    pathname_ = pathname;
  }

  //======================================================================
  void MemoryTrackerLinux::preEventProcessing(const Event & e)
  {
    eventId_ = e.id();
    evtData_ = procInfo_.getCurrentData();
  }

  void MemoryTrackerLinux::postEventProcessing(const Event &)
  {
    ++evtCount_;

    auto const data   = procInfo_.getCurrentData();
    auto const deltas = data-evtData_;

    if (evtCount_ <= numToSkip_) { return; }

    eventTable_.insert( eventId_.run(),
                        eventId_.subRun(),
                        eventId_.event(),
                        data.at(LinuxProcData::VSIZE),
                        deltas.at(LinuxProcData::VSIZE),
                        data.at(LinuxProcData::RSS),
                        deltas.at(LinuxProcData::RSS) );

    if ( includeMallocInfo_ ) {
      auto minfo = LinuxMallInfo().get();
      eventHeapTable_->insert( eventTable_.lastRowid(),
                               minfo.arena,
                               minfo.ordblks,
                               minfo.keepcost,
                               minfo.hblkhd,
                               minfo.hblks,
                               minfo.uordblks,
                               minfo.fordblks );
    }

  }

  //======================================================================
  void MemoryTrackerLinux::preModule(ModuleDescription const &)
  {
    modData_ = procInfo_.getCurrentData();
  }

  void MemoryTrackerLinux::postModule(ModuleDescription const & md)
  {
    auto const data   = procInfo_.getCurrentData();
    auto const deltas = data-modData_;

    if (evtCount_ < numToSkip_) { return; }

    moduleTable_.insert( eventId_.run(),
                         eventId_.subRun(),
                         eventId_.event(),
                         pathname_+":"s+md.moduleLabel()+":"s+md.moduleName(),
                         data.at(LinuxProcData::VSIZE),
                         deltas.at(LinuxProcData::VSIZE),
                         data.at(LinuxProcData::RSS),
                         deltas.at(LinuxProcData::RSS) );

    if ( includeMallocInfo_ ) {
      auto minfo = LinuxMallInfo().get();
      moduleHeapTable_->insert( moduleTable_.lastRowid(),
                                minfo.arena,
                                minfo.ordblks,
                                minfo.keepcost,
                                minfo.hblkhd,
                                minfo.hblks,
                                minfo.uordblks,
                                minfo.fordblks );
    }

  }

  //======================================================================
  void MemoryTrackerLinux::postEndJob()
  {

    if ( printSummary_.none() ) return;

    std::ostringstream msgOss;

    msgOss << ::rule << "\n\n";
    msgOss << std::left << "MemoryTracker SUMMARY (all numbers in units of Mbytes)"
           << "\n\n";

    if ( printSummary_.test(GENERAL) ) {
      generalSummary_( msgOss );
    }
    if ( printSummary_.test(EVENT)   ) {
      eventSummary_  ( msgOss, "DeltaVsize",  "Events increasing Vsize (Mbytes)" );
      eventSummary_  ( msgOss, "Vsize"     ,  "Events with large Vsize (Mbytes)" );
    }
    if ( printSummary_.test(MODULE) ) {
      moduleSummary_ ( msgOss, "DeltaVsize", "Modules increasing Vsize (Mbytes)" );
      moduleSummary_ ( msgOss, "Vsize"     , "Modules with large Vsize (Mbytes)" );
    }

    msgOss << ::rule << "\n";

    mf::LogAbsolute("MemoryTracker") << msgOss.str();

  }

  //======================================================================
  // Private member functions

  std::bitset<MemoryTrackerLinux::ntypes>
  MemoryTrackerLinux::setbits_( std::vector<std::string> const& pset )
  {

    std::bitset<ntypes> bset;

    if ( pset.empty() ) {
      return bset;
    }

    std::set<std::string> const pset_bits ( pset.cbegin(), pset.cend() );

    if ( pset_bits.find("*") != pset_bits.cend() ) {
      if ( pset_bits.size() == 1 ) bset.set();
      else throw cet::exception("CONFIG") << "The summary option '*' cannot be included with any other options.";
    }

    if ( pset_bits.find( "general" ) != pset_bits.cend() ) bset.set(GENERAL);
    if ( pset_bits.find( "event"   ) != pset_bits.cend() ) bset.set(EVENT  );
    if ( pset_bits.find( "module"  ) != pset_bits.cend() ) bset.set(MODULE );

    return bset;
  }

  //======================================================================
  bool MemoryTrackerLinux::checkMallocConfig_( std::string const& dbfilename,
                                               bool const include )
  {
    if ( include && dbfilename.empty() ) {
      throw cet::exception("CONFIG") << "\n'includeMallocInfo : true' is valid only if a nonempty db filename is specified:\n\n"
                                     << " MemoryTracker: {\n"
                                     << "   includeMallocInfo : true\n"
                                     << "   filename          : \"your_filename.db\"\n"
                                     << " }\n\n";
    }
    return include;
  }

  //======================================================================
  void MemoryTrackerLinux::generalSummary_( std::ostringstream& oss )
  {
    summaryTable_.flush();

    const std::vector<std::string> steps = sqlite::getUniqueEntries<std::string>( dbMgr_.get(), "Summary", "ProcessStep" );
    const std::vector<std::string> mods  = sqlite::getUniqueEntries<std::string>( dbMgr_.get(), "Summary", "ModuleId"    );

    // Calculate column widths
    std::size_t sWidth(0), mWidth(0);
    std::for_each( steps.cbegin(), steps.cend(), [&sWidth](auto const& s) { sWidth = std::max( sWidth, s.size() ); } );
    std::for_each( mods .cbegin(), mods .cend(), [&mWidth](auto const& s) { mWidth = std::max( mWidth, s.size() ); } );

    const std::string rule = std::string(sWidth+2+mWidth+2+2*12,'=');

    oss << "  Peak virtual memory usage (VmPeak): " << procInfo_.getVmPeak() << " Mbytes"
        << "\n\n"
        << std::setw(sWidth+2) << "ProcessStep"
        << std::setw(mWidth+2) << "Module ID/Event No."
        << boost::format(" %_=10s ") % "\u0394 Vsize"
        << boost::format(" %_=10s ") % "\u0394 RSS"
        << "\n" << rule << "\n";

    std::string cachedStep = "";
    std::size_t i(0);
    for ( auto & entry : sqlite::query_db( dbMgr_.get(), "SELECT * FROM Summary") ) {

      std::string step;
      std::string module;
      double dv;
      double drss;

      entry >> step >> module >> dv >> drss;

      if ( cachedStep != step ) {
        cachedStep = step;
        if ( i ) {
          oss << std::string(sWidth+2+mWidth+2+2*12,'-') << "\n";
        }
      }

      oss << std::setw(sWidth+2) << std::left << step;
      oss << std::setw(mWidth+2) << std::left << module;
      oss << sanitizeZero<10>(   dv );
      oss << sanitizeZero<10>( drss );
      oss << "\n";
      ++i;

    }
    oss << "\n";
  }

  //======================================================================
  void MemoryTrackerLinux::eventSummary_( std::ostringstream& oss,
                                          std::string const& column,
                                          std::string const& header)
  {
    eventTable_.flush();

    eventDataList_t evtList;
    for ( auto & entry : sqlite::query_db( dbMgr_.get(),
                                           "SELECT rowid,* FROM EventInfo "s+
                                           "WHERE "s+column+" > 0 ORDER BY "s+column+" DESC LIMIT 5"s,
                                           false ) ) {
      auto evtData = convertToEvtIdData( entry );
      std::get<std::size_t>( evtData ) += numToSkip_;
      evtList.push_back( evtData );
    }

    std::size_t noWidth(0), evtWidth(0);
    for ( auto const & data : evtList ) {
      auto const evtCount = std::get<std::size_t>( data );
      noWidth  = std::max( noWidth , " ["s.size()+std::to_string(evtCount).size()+"]"s.size() );
      evtWidth = std::max( evtWidth, std::get<std::string>( data ).size() );
    }

    std::size_t const width = std::max( header.size(), noWidth+evtWidth );

    const std::string rule = std::string(width+4+4*12,'=');

    oss << "\n"
        << banners::vsize_rss( header, width+4, 10) << "\n"
        << rule << "\n";

    if ( evtList.empty() ) {
      oss << "  [[ no events ]] " << "\n";
    }

    for ( auto const & data : evtList ) {
      std::ostringstream preamble;
      preamble << " [" << std::setw( noWidth-3) << std::left << std::get<std::size_t>( data ) << "]  "
               <<         std::setw(evtWidth+2) << std::left << std::get<std::string>( data );
      oss << std::setw(width+4) << preamble.str() << std::get<MemData>( data ) << "\n";
    }
    oss << "\n";
  }

  //======================================================================
  void MemoryTrackerLinux::moduleSummary_( std::ostringstream& oss,
                                           std::string const& column,
                                           std::string const& header )
  {
    moduleTable_.flush();

    const std::vector<std::string> mods = sqlite::getUniqueEntries<std::string>( dbMgr_.get(), "ModuleInfo", "PathModuleId" );

    // Fill map, which will have form
    //
    //   mod1 : event1_data [ std::vector<std::string> ]
    //          event2_data [ "" ]
    //          event3_data [ "" ]
    //   mod2 : event1_data [ std::vector<std::string> ], etc.
    //

    orderedMap_t<modName_t,eventDataList_t> modMap;

    for ( auto const & mod : mods ) {

      std::string const ddl =
        "CREATE TABLE temp.tmpModTable AS "s +
        "SELECT * FROM ModuleInfo WHERE PathModuleId='"s+mod+"'"s;

      sqlite::exec( dbMgr_.get(), ddl );

      std::string const columns = "rowid,Run,Subrun,Event,Vsize,DeltaVsize,RSS,DeltaRSS";

      eventDataList_t evtList;
      for ( auto & entry : sqlite::query_db( dbMgr_.get(),
                                             "SELECT "s+columns+" FROM temp.tmpModTable "s+
                                             "WHERE "+column+" > 0 ORDER BY "s+column+" DESC LIMIT 5"s,
                                             false ) ) {
        auto evtData = convertToEvtIdData( entry );
        std::get<std::size_t>( evtData ) += numToSkip_;
        evtList.emplace_back( evtData );
      }
      modMap.emplace_back( mod, evtList );

      sqlite::dropTable( dbMgr_.get(), "temp.tmpModTable" );

    }

    // Calculate widths
    std::size_t modWidth(0), noWidth(0), evtWidth(0);
    for ( auto const & mod : modMap ) {
      modWidth = std::max( modWidth, mod.first.size() );
      for ( auto const & evtInfo : mod.second ) {
        auto const & evtCount = std::get<std::size_t>( evtInfo );
        noWidth  = std::max( noWidth , " ["s.size()+std::to_string( evtCount ).size()+"]"s.size() );
        evtWidth = std::max( evtWidth, std::get<std::string>( evtInfo ).size() );
      }
    }

    std::size_t const width = std::max( {header.size(), modWidth, noWidth+evtWidth} );
    std::string const rule  = std::string(width+4+4*12,'=');

    oss << "\n"
        << banners::vsize_rss( header, width+2, 10 ) << "\n"
        << rule   << "\n";

    for ( auto itMod = modMap.cbegin(); itMod != modMap.cend() ; ++itMod ) {
      oss << itMod->first << "\n";

      if ( itMod->second.empty() ) {
        oss << " [[ no events ]] " << "\n";
      }

      for ( auto const & evtInfo : itMod->second ) {
        std::ostringstream preamble;
        preamble << " [" << std::setw( noWidth-3) << std::left << std::get<std::size_t>( evtInfo ) << "]  "
                 <<         std::setw(evtWidth+2) << std::left << std::get<std::string>( evtInfo );
        oss << std::setw(width+2) << preamble.str() << std::get<MemData>( evtInfo ) << "\n";
      }

      if ( std::next(itMod) != modMap.cend() ) {
        oss << std::string(width+4+4*12,'-') << "\n";
      }

    }
    oss << "\n" ;

  }

} // art
