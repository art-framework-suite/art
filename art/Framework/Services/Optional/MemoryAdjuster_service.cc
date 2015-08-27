// ======================================================================
//
// SimpleMemoryCheck
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "art/Utilities/MallocOpts.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Name.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

//====================================================================================

namespace {
  using namespace fhicl;
  struct Config {
    Atom<int>  m_mmap_max { Name("M_MMAP_MAX"), -1};
    Atom<int>  m_trim_thr { Name("M_TRIM_THRESHOLD"), -1};
    Atom<int>  m_top_pad  { Name("M_TOP_PAD"), -1};
    Atom<int>  m_mmap_thr { Name("M_MMAP_THRESHOLD"), -1};
    Atom<bool> dump       { Name("dump"), false };
  };
}

namespace art {

  class ActivityRegistry;

  class MemoryAdjuster {
  public:

    using Parameters = ServiceTable<Config>;
    MemoryAdjuster(ServiceTable<Config> const & config [[gnu::unused]], ActivityRegistry &) {
#ifndef __linux__
      mf::LogAbsolute("MemoryAdjuster") << "\n"
                                        << "Service not supported for this operating system.\n"
                                        << "If desired, please log an issue with:\n\n"
                                        << "https://cdcvs.fnal.gov/redmine/projects/cet-is/issues/new\n\n";
#else
      typedef art::MallocOpts::opt_type opt_type;
      art::MallocOptionSetter & mopts = art::getGlobalOptionSetter();
      opt_type const
        p_mmap_max = config().m_mmap_max(),
        p_trim_thr = config().m_trim_thr(),
        p_top_pad  = config().m_top_pad(),
        p_mmap_thr = config().m_mmap_thr();
      if (p_mmap_max >= 0) { mopts.set_mmap_max(p_mmap_max); }
      if (p_trim_thr >= 0) { mopts.set_trim_thr(p_trim_thr); }
      if (p_top_pad  >= 0) { mopts.set_top_pad (p_top_pad ); }
      if (p_mmap_thr >= 0) { mopts.set_mmap_thr(p_mmap_thr); }
      mopts.adjustMallocParams();
      if ( mopts.hasErrors() ) {
        mf::LogWarning("MemoryCheck")
          << "ERROR: Problem with setting malloc options\n"
          << mopts.error_message();
      }
      if ( config().dump() ) {
        art::MallocOpts mo = mopts.get();
        mf::LogWarning("MemoryCheck") << "Malloc options: " << mo << "\n";
      }
#endif
    } // c'tor
  };

}  // art

// ======================================================================

// The DECLARE macro call should be moved to the header file, should you
// create one.
DECLARE_ART_SERVICE(art::MemoryAdjuster, LEGACY)
DEFINE_ART_SERVICE(art::MemoryAdjuster)

// ======================================================================
