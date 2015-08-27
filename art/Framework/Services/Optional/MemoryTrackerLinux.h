#ifndef art_Framework_Services_Optional_MemoryTrackerLinux_h
#define art_Framework_Services_Optional_MemoryTrackerLinux_h

// ======================================================================
//
// MemoryTracker
//
// ======================================================================

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Optional/detail/LinuxProcData.h"
#include "art/Framework/Services/Optional/detail/LinuxProcMgr.h"
#include "art/Framework/Services/Optional/detail/MemoryTrackerLinuxCallbackPair.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "art/Ntuple/Ntuple.h"
#include "art/Ntuple/sqlite_DBmanager.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "cetlib/exempt_ptr.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Sequence.h"

#include <bitset>
#include <memory>
#include <tuple>

namespace art {

  class MemoryTracker{
  public:

    struct Config {
      using Name = fhicl::Name;
      fhicl::Atom<unsigned> ignoreTotal { Name("ignoreTotal"), 1 };
      fhicl::Sequence<std::string> printSummaries { Name("printSummaries"), { "general", "event", "module" } };
      fhicl::Atom<std::string> filename { Name("filename"), "" };
      fhicl::Atom<bool> includeMallocInfo { Name("includeMallocInfo"), false };
    };

    using Parameters = ServiceTable<Config>;
    MemoryTracker(ServiceTable<Config> const &, ActivityRegistry &);

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

    detail::LinuxProcMgr procInfo_;

    // Options
    unsigned numToSkip_;
    std::bitset<ntypes> printSummary_;
    sqlite::DBmanager dbMgr_;
    bool     includeMallocInfo_;

    std::string pathname_;
    detail::LinuxProcData::proc_array evtData_;
    art::EventID eventId_;
    std::size_t  evtCount_;

    detail::LinuxProcData::proc_array modData_;

    template<unsigned SIZE>
    using name_array = ntuple::name_array<SIZE>;

    name_array<4u> summaryTuple_;
    name_array<7u> eventTuple_;
    name_array<8u> moduleTuple_;
    name_array<8u> eventHeapTuple_;
    name_array<8u> moduleHeapTuple_;

    using memSummary_t = ntuple::Ntuple<std::string,std::string,double,double>;
    using memEvent_t   = ntuple::Ntuple<uint32_t,uint32_t,uint32_t,double,double,double,double>;
    using memModule_t  = ntuple::Ntuple<uint32_t,uint32_t,uint32_t,std::string,double,double,double,double>;
    using memHeap_t    = ntuple::Ntuple<sqlite_int64,int,int,int,int,int,int,int>;

    memSummary_t summaryTable_;
    memEvent_t   eventTable_;
    memModule_t  moduleTable_;
    std::unique_ptr<memHeap_t> eventHeapTable_;
    std::unique_ptr<memHeap_t> moduleHeapTable_;

    template<typename T>
    using CallbackPair      = detail::CallbackPair<T>;
    using SourceSummaryType = detail::SourceSummaryType;
    using ModuleSummaryType = detail::ModuleSummaryType;

    CallbackPair<SourceSummaryType> evtSource_;
    CallbackPair<ModuleSummaryType> modConstruction_;
    CallbackPair<ModuleSummaryType> modBeginJob_;
    CallbackPair<ModuleSummaryType> modEndJob_;
    CallbackPair<ModuleSummaryType> modBeginRun_;
    CallbackPair<ModuleSummaryType> modEndRun_;
    CallbackPair<ModuleSummaryType> modBeginSubRun_;
    CallbackPair<ModuleSummaryType> modEndSubRun_;

  }; // MemoryTracker

}  // art

#endif // art_Framework_Services_Optional_MemoryTracker_h

// Local variables:
// mode: c++
// End:
