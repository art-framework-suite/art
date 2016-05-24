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
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "cetlib/exempt_ptr.h"

#include <string>

namespace art {

  namespace detail {

    using memSummary_t = ntuple::Ntuple<std::string,std::string,double,double>;

    class CallbackPair {
    public:

      CallbackPair(memSummary_t& summaryTable,
                   LinuxProcMgr const & procMgr,
                   std::size_t  const & evtCounter,
                   std::string  const & processStep)
        : summaryTable_{&summaryTable}
        , procMgr_{&procMgr}
        , counter_{&evtCounter}
        , processStep_ {processStep}
      {}

      void pre(ModuleDescription const&)
      {
        baseline_ = procMgr_->getCurrentData();
      }

      template<typename ... ARGS>
      void post(ModuleDescription const& md)
      {
        deltas_ = procMgr_->getCurrentData()-baseline_;
        summaryTable_->insert(processStep_,
                              md.moduleLabel()+":"s+md.moduleName(),
                              deltas_.at(LinuxProcData::VSIZE),  /// FIXME!!!!
                              deltas_.at(LinuxProcData::RSS ) );
      }

    private:
      cet::exempt_ptr<memSummary_t> summaryTable_;
      cet::exempt_ptr<LinuxProcMgr const> procMgr_;
      cet::exempt_ptr<std::size_t const>  counter_;
      std::string processStep_;

      LinuxProcData::proc_array baseline_ {{0.}};
      LinuxProcData::proc_array deltas_ {{0.}};
    };

  } // detail
} // art

#endif /* art_Framework_Services_Optional_detail_MemoryTrackerLinuxCallbackPair_h */

// Local variables:
// mode: c++
// End:
