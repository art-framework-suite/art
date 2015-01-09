#ifndef art_Framework_Services_Optional_detail_MemoryTrackerLinuxCallbackPair_h
#define art_Framework_Services_Optional_detail_MemoryTrackerLinuxCallbackPair_h
// ======================================================================
//
// MemoryTrackerLinuxCallbackPair
//
// ======================================================================

#include "art/Framework/Services/Optional/detail/LinuxProcData.h"
#include "art/Framework/Services/Optional/detail/LinuxProcMgr.h"
#include "art/Ntuple/Ntuple.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "cetlib/exempt_ptr.h"

#include <string>

using namespace ntuple;

namespace art {

  namespace detail {

    using memSummary_t = Ntuple<std::string,std::string,double,double>;

    class SourceSummaryType;
    class ModuleSummaryType;

    template <typename T>
    class CallbackPair {
    public:

      CallbackPair(memSummary_t& summaryTable,
                   LinuxProcMgr const & procMgr,
                   std::size_t  const & evtCounter,
                   std::string  const & processStep)
        : summaryTable_( &summaryTable )
        , procMgr_     ( &procMgr      )
        , counter_     ( &evtCounter   )
        , processStep_ (  processStep  )
      {}

      template<typename ... ARGS>
      void pre (ARGS const & ... )
      {
        baseline_ = procMgr_->getCurrentData();
      }

      template<typename ... ARGS>
      void post(ARGS const & ... )
      {
        deltas_ = procMgr_->getCurrentData()-baseline_;
        summaryTable_->insert( processStep_,
                               "[ EVT ] #: "s +std::to_string( *counter_ ),
                               deltas_.at(LinuxProcData::VSIZE),
                               deltas_.at(LinuxProcData::RSS ) );
      }

    private:

      cet::exempt_ptr<memSummary_t>       summaryTable_;
      cet::exempt_ptr<const LinuxProcMgr> procMgr_;
      cet::exempt_ptr<const std::size_t>  counter_;
      std::string processStep_;

      LinuxProcData::proc_array baseline_;
      LinuxProcData::proc_array deltas_;

    };

    template<>
    template<>
    void
    CallbackPair<ModuleSummaryType>::post<ModuleDescription const &>( ModuleDescription const & md ) {
      deltas_ = procMgr_->getCurrentData()-baseline_;
      summaryTable_->insert( processStep_,
                             "[ MOD ] "s+md.moduleLabel()+":"s+md.moduleName(),
                             deltas_.at(LinuxProcData::VSIZE),
                             deltas_.at(LinuxProcData::RSS ) );
    }

  } // detail
} // art

#endif // art_Framework_Services_Optional_detail_MemoryTrackerLinuxCallbackPair_h

// Local variables:
// mode: c++
// End:
