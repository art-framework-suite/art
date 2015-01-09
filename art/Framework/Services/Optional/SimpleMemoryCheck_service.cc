// ======================================================================
//
// SimpleMemoryCheck
//
// ======================================================================

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Optional/detail/constrained_multimap.h"
#include "art/Framework/Services/Optional/detail/LinuxMallInfo.h"
#include "art/Framework/Services/Optional/detail/LinuxProcData.h"
#include "art/Framework/Services/Optional/detail/LinuxProcMgr.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Utilities/MallocOpts.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

extern "C" {
#include <fcntl.h>
#include <unistd.h>
}

#include <cstdlib>
#include <iterator>
#include <tuple>
#include <type_traits>

#ifdef __linux__
#include <cstdio>
#endif

using namespace art::detail;

//===================================================================
// Helpers

namespace {

  using proc_array = LinuxProcData::proc_array;

  //=================================================================
  struct print_proc_info {

    explicit print_proc_info( proc_array const & data_array,
                              proc_array const & delta_array )
      : data_( data_array )
      , deltas_( delta_array )
    {}

    proc_array data_;
    proc_array deltas_;

  };

  // corresponding ostream operator
  // .. T&& is a universal reference (can be lvalue or rvalue).  Use
  // .. std::forward<T> to forward ml as an lvalue or rvalue
  // .. reference, depending on whether the 'ml' was an lvalue or
  // .. rvalue.  Return type must be 'decltype(auto)' so that
  // .. correctly reference-qualified return type is specified.

  template <typename T>
  decltype(auto) operator<< ( T&& ml, print_proc_info && info ) {
    ml << " VSIZE " << info.data_.at(LinuxProcData::VSIZE) << " " << info.deltas_.at(LinuxProcData::VSIZE)
       << " RSS "   << info.data_.at(LinuxProcData::RSS)   << " " << info.deltas_.at(LinuxProcData::RSS);
    return std::forward<T>(ml);
  }

  //====================================================================
  template <typename MAP>
  void filter_null_keys ( MAP & constrainedMMap ) {

    auto iter = constrainedMMap.cbegin();
    while ( iter != constrainedMMap.cend() ) {
      auto next_iter = std::next( iter );
      if ( iter->first == 0. ) {
        constrainedMMap.erase( iter );
      }
      iter = next_iter;
    }

  }

} // anon. namespace

//====================================================================================

namespace art {

  class SimpleMemoryCheck {
  public:

    SimpleMemoryCheck(fhicl::ParameterSet const &, ActivityRegistry &);

    void postSource();

    void preEventProcessing (Event const &);
    void postEventProcessing(Event const &);

    void postModuleBeginJob(ModuleDescription const &);
    void postModuleConstruction(ModuleDescription const &);

    void preModule (ModuleDescription const &);
    void postModule(ModuleDescription const &);

    void postEndJob();

  private:

    void print         (std::string const & type, std::string const & mdlabel, std::string const & mdname ) const;
    void updateAndPrint(std::string const & type, std::string const & mdlabel, std::string const & mdname );

    std::unique_ptr<LinuxProcMgr> procInfo_;

    // Options
    unsigned numToSkip_;
    bool     truncateSummary_;
    bool     showMallocInfo_;
    bool     oncePerEventMode_;
    bool     moduleSummaryRequested_;

    LinuxProcData::proc_array currData_;
    LinuxProcData::proc_array deltaData_;
    LinuxProcData::proc_array maxData_;
    bool maxUpdated_;

    // Event level
    // .. necessary to have separate data member 'evtData_' for events
    // .. since update() can be called after all of the modules are
    // .. executed, and then after the event, thus erasing the "delta" values.
    art::EventID currentEventID_;
    unsigned     evtCount_;
    LinuxProcData::proc_array evtData_;

    // .. Significant event memory loggers
    using dv_mapped_type = std::pair<art::EventID,double>;
    constrained_multimap<3u,double,art::EventID>        sigEventMap_Vsize_;
    constrained_multimap<5u,double,dv_mapped_type> sigEventMap_deltaVsize_;

    void update();
    void updateEventStats(art::EventID const & e);

    // Module level
    // .. Module summary statistics
    struct SignificantModule_ {
      unsigned postEarlyCount;
      double   totalDeltaVsize;
      double   maxDeltaVsize;
      art::EventID maxDeltaVeventID;
      double   totalEarlyVsize;
      double   maxEarlyVsize;

      SignificantModule_()
        : postEarlyCount()
        , totalDeltaVsize()
        , maxDeltaVsize()
        , totalEarlyVsize()
        , maxEarlyVsize()
      {}

    };

    std::map<std::string, SignificantModule_> modules_;
    void updateModuleStats(SignificantModule_ & m, double const dv);

  }; // SimpleMemoryCheck

}  // art

// ======================================================================

namespace art {

  SimpleMemoryCheck::SimpleMemoryCheck(const fhicl::ParameterSet & iPS,
                                       ActivityRegistry & iReg) :
#ifdef __linux__
    procInfo_        ( std::make_unique<LinuxProcMgr>() ) ,
#endif
    numToSkip_     (iPS.get<unsigned>("ignoreTotal"     , 1    ))
    , truncateSummary_ (iPS.get<bool>("truncateSummary" , true ))
    , showMallocInfo_  (iPS.get<bool>("showMallocInfo"  , false))
    , oncePerEventMode_(iPS.get<bool>("oncePerEventMode", false))
    , moduleSummaryRequested_(iPS.get<bool>("moduleMemorySummary", false))
    , maxUpdated_(false)
    , evtCount_()

  {
#ifndef __linux__
    mf::LogAbsolute("MemoryTracker") << "\n"
                                     << "Service currently not supported for this operating system.\n"
                                     << "If desired, please log an issue with:\n\n"
                                     << "https://cdcvs.fnal.gov/redmine/projects/cet-is/issues/new\n\n";
#else
    if (!oncePerEventMode_) { // default, prints on increases
      iReg.sPostSource.watch(this,&SimpleMemoryCheck::postSource);
      iReg.sPostModuleConstruction.watch(this,&SimpleMemoryCheck::postModuleConstruction);
      iReg.sPostModuleBeginJob.watch(this,&SimpleMemoryCheck::postModuleBeginJob);
      iReg.sPostProcessEvent.watch(this,&SimpleMemoryCheck::postEventProcessing);
      iReg.sPostModule.watch(this,&SimpleMemoryCheck::postModule);
      iReg.sPostEndJob.watch(this,&SimpleMemoryCheck::postEndJob);
    }
    else {
      iReg.sPostProcessEvent.watch(this,&SimpleMemoryCheck::postEventProcessing);
      iReg.sPostEndJob.watch(this,&SimpleMemoryCheck::postEndJob);
    }
    if (moduleSummaryRequested_) {
      iReg.sPreProcessEvent.watch(this,&SimpleMemoryCheck::preEventProcessing);
      iReg.sPreModule.watch(this,&SimpleMemoryCheck::preModule);
      if (oncePerEventMode_) {
        iReg.sPostModule.watch(this,&SimpleMemoryCheck::postModule);
      }
    }

    typedef art::MallocOpts::opt_type opt_type;
    art::MallocOptionSetter & mopts = art::getGlobalOptionSetter();
    opt_type
      p_mmap_max = iPS.get<int>("M_MMAP_MAX", -1),
      p_trim_thr = iPS.get<int>("M_TRIM_THRESHOLD", -1),
      p_top_pad  = iPS.get<int>("M_TOP_PAD", -1),
      p_mmap_thr = iPS.get<int>("M_MMAP_THRESHOLD", -1);
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
    if (iPS.get<bool>("dump", false) == true) {
      art::MallocOpts mo = mopts.get();
      mf::LogWarning("MemoryCheck") << "Malloc options: " << mo << "\n";
    }
#endif
  }

  void SimpleMemoryCheck::postSource()
  {
    updateAndPrint("module", "source", "source");
  }

  void SimpleMemoryCheck::postModuleConstruction(const ModuleDescription & md)
  {
    updateAndPrint("ctor", md.moduleLabel(), md.moduleName());
  }

  void SimpleMemoryCheck::postModuleBeginJob(const ModuleDescription & md)
  {
    updateAndPrint("beginJob", md.moduleLabel(), md.moduleName());
  }

  void SimpleMemoryCheck::postEndJob()
  {

    filter_null_keys( sigEventMap_deltaVsize_ );
    filter_null_keys( sigEventMap_Vsize_      );

    {

      // In its own block since message is logged upon destruction of
      // 'ml'.

      mf::LogAbsolute ml("MemoryReport");

      ml << "MemoryReport> Peak virtual size " << maxData_.at(LinuxProcData::VSIZE) << " Mbytes";

      ml << "\nKey events increasing vsize:\n";
      if ( sigEventMap_deltaVsize_.empty() ) {
        ml << "[none]\n";
      }
      else {
        auto const & map = sigEventMap_deltaVsize_;
        unsigned i(1);
        for ( auto it = map.crbegin(); it != map.crend(); ++it, ++i) {
          if ( truncateSummary_ && i>map.unique_key_limit() ) break;
          auto idVsizePair = it->second;
          ml << "["<<i<<"] "
             << std::get<art::EventID>( idVsizePair )
             << "  vsize = " << std::get<double>( idVsizePair )
             << " deltaVsize = "<< it->first
             << "\n";
        }
      }

      ml << "\nKey events with largest vsize:\n";
      if ( sigEventMap_Vsize_.empty() ) {
        ml << "[none]\n";
      }
      else {
        auto const & map = sigEventMap_Vsize_;
        unsigned i(1);
        for ( auto it = map.crbegin(); it != map.crend(); ++it, ++i ) {
          if ( truncateSummary_ && i>map.unique_key_limit() ) break;
          auto   const& eid   = it->second;
          double const& vsize = it->first;
          ml << "["<<i<<"] " << eid << "  vsize = " << vsize << "\n";
        }
      }
    }

    if (moduleSummaryRequested_) {

      // at end of if block, mmr is destroyed, causing message to be logged
      mf::LogAbsolute mmr("ModuleMemoryReport");
      mmr << "ModuleMemoryReport> Each line has module label and:\n"
          << "  (after early ignored events)\n"
          << "    count of times module executed; average increase in vsize\n"
          << "    maximum increase in vsize; event on which maximum occurred\n"
          << "  (during early ignored events)\n"
          << "    total and maximum vsize increases\n\n";

      for ( const auto& mod : modules_ ) {
        SignificantModule_ const & m = mod.second;

        if (m.totalDeltaVsize == 0 && m.totalEarlyVsize == 0) continue;

        mmr << mod.first << ": n = " << m.postEarlyCount;

        if (m.postEarlyCount > 0) {
          mmr << " avg = " << m.totalDeltaVsize / m.postEarlyCount;
        }

        mmr <<  " max = " << m.maxDeltaVsize << " " << m.maxDeltaVeventID;

        if (m.totalEarlyVsize > 0) {
          mmr << " early total: " << m.totalEarlyVsize
              << " max: " << m.maxEarlyVsize;
        }

        mmr << "\n";
      }
    } // end of if; mmr goes out of scope; log message is queued
  } // postEndJob

  void SimpleMemoryCheck::preEventProcessing(const art::Event & ev)
  {
    currentEventID_ = ev.id();
  }

  void SimpleMemoryCheck::postEventProcessing(const Event & e)
  {
    updateAndPrint("event", "", "");
    updateEventStats(e.id());
    ++evtCount_;
  }

  void SimpleMemoryCheck::preModule(const ModuleDescription &)
  {
    update();
  }

  void SimpleMemoryCheck::postModule(const ModuleDescription & md)
  {
    if (!oncePerEventMode_) {
      updateAndPrint("module", md.moduleLabel(), md.moduleName());
    }
    else if (moduleSummaryRequested_) {
      update();
    }
    if (moduleSummaryRequested_) {
      std::string const label =  md.moduleLabel();
      updateModuleStats( modules_[label], deltaData_.at(LinuxProcData::VSIZE) );
    }
  }


  //================================================================
  // Private member functions

  void SimpleMemoryCheck::update()
  {
    deltaData_  = procInfo_->getCurrentData() - currData_;
    currData_  += deltaData_;
    maxUpdated_ = false;
    if ( currData_ > maxData_ ) {
      maxData_ = currData_;
      maxUpdated_ = true;
    }
  }

  void SimpleMemoryCheck::print(const std::string & type,
                                const std::string & mdlabel,
                                const std::string & mdname) const
  {

    if ( !maxUpdated_ && !oncePerEventMode_ ) return;
    if ( evtCount_ < numToSkip_ ) return;

    mf::LogWarning mf("MemoryCheck");
    mf << "MemoryCheck: " << type << " " << mdname << ":" << mdlabel
       << print_proc_info( currData_, deltaData_ );
    if ( showMallocInfo_ ) {
      mf << LinuxMallInfo();
    }
    mf << "\n";
  }

  void SimpleMemoryCheck::updateAndPrint(const std::string & type,
                                         const std::string & mdlabel,
                                         const std::string & mdname)
  {
    update();
    print(type, mdlabel, mdname);
    maxData_ = std::max( maxData_, currData_ );
  }

  void SimpleMemoryCheck::updateEventStats(art::EventID const & e)
  {
    auto tmpDelta = currData_-evtData_;
    evtData_      = currData_;

    if (evtCount_ < numToSkip_) { return; }

    double const & vsize      = evtData_.at(LinuxProcData::VSIZE);
    double const & deltaVsize = tmpDelta.at(LinuxProcData::VSIZE);

    sigEventMap_Vsize_     .maybe_emplace( vsize, e );
    sigEventMap_deltaVsize_.maybe_emplace( deltaVsize, std::make_pair(e, vsize ) );
  }   // updateEventStats

  void
  SimpleMemoryCheck::updateModuleStats(SignificantModule_ & m, double const dv)
  {

    if (evtCount_ < numToSkip_) {
      m.totalEarlyVsize += dv;
      m.maxEarlyVsize = std::max( m.maxEarlyVsize, dv );
    }
    else {
      ++m.postEarlyCount;
      m.totalDeltaVsize += dv;
      if (dv > m.maxDeltaVsize)  {
        m.maxDeltaVsize    = dv;
        m.maxDeltaVeventID = currentEventID_;
      }
    }
  } //updateModuleStats

} // art

// ======================================================================

// The DECLARE macro call should be moved to the header file, should you
// create one.
DECLARE_ART_SERVICE(art::SimpleMemoryCheck, LEGACY)
DEFINE_ART_SERVICE(art::SimpleMemoryCheck)

// ======================================================================
