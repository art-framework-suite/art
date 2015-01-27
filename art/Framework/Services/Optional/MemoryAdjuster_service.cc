// ======================================================================
//
// SimpleMemoryCheck
//
// ======================================================================

#include "art/Framework/Services/Optional/MemoryTracker.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Utilities/MallocOpts.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

//====================================================================================

namespace art {

  class ActivityRegistry;

  class MemoryAdjuster {
  public:

    MemoryAdjuster(fhicl::ParameterSet const & iPS, ActivityRegistry &) {
#ifndef __linux__
      mf::LogAbsolute("MemoryTracker") << "\n"
                                       << "Service not supported for this operating system.\n"
                                       << "If desired, please log an issue with:\n\n"
                                       << "https://cdcvs.fnal.gov/redmine/projects/cet-is/issues/new\n\n";
#else
      
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
      if ( mopts.hasErrors() ) {
        mf::LogWarning("MemoryCheck")
          << "ERROR: Problem with setting malloc options\n"
          << mopts.error_message();
      }
      if (iPS.get<bool>("dump", false) == true) {
        art::MallocOpts mo = mopts.get();
        mf::LogWarning("MemoryCheck") << "Malloc options: " << mo << "\n";
      }
      
    } // c'tor
#endif
  };

}  // art

// ======================================================================

// The DECLARE macro call should be moved to the header file, should you
// create one.
DECLARE_ART_SERVICE(art::MemoryAdjuster, LEGACY)
DEFINE_ART_SERVICE(art::MemoryAdjuster)

// ======================================================================
