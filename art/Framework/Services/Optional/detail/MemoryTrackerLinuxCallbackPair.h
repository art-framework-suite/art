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

    class CallbackPair {
    public:

      using otherInfo_t = ntuple::Ntuple<std::string,std::string,std::string,double,double>;

      CallbackPair(otherInfo_t& table,
                   LinuxProcMgr const& procMgr,
                   std::string const& processStep)
        : table_{&table}
        , procMgr_{&procMgr}
        , processStep_{processStep}
      {}

      void pre(ModuleDescription const&)
      {
        baseline_ = procMgr_->getCurrentData();
      }

      void post(ModuleDescription const& md)
      {
        deltas_ = procMgr_->getCurrentData()-baseline_;
        sqlite::insert_into(*table_).values(processStep_,
                                            md.moduleLabel(),
                                            md.moduleName(),
                                            deltas_[LinuxProcData::VSIZE],
                                            deltas_[LinuxProcData::RSS]);
      }

    private:
      cet::exempt_ptr<otherInfo_t> table_;
      cet::exempt_ptr<LinuxProcMgr const> procMgr_;
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
