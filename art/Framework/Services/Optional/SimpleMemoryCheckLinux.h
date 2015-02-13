#ifndef art_Framework_Services_Optional_SimpleMemoryCheckLinux_h
#define art_Framework_Services_Optional_SimpleMemoryCheckLinux_h

// ======================================================================
//
// SimpleMemoryCheck
//
// ======================================================================

#include "art/Framework/Services/Optional/detail/constrained_multimap.h"
#include "art/Framework/Services/Optional/detail/LinuxProcData.h"
#include "art/Framework/Services/Optional/detail/LinuxProcMgr.h"
#include "art/Persistency/Provenance/EventID.h"

#include <memory>

namespace art   {
  class ActivityRegistry;
  class Event;
  class ModuleDescription;
}
namespace fhicl {
  class ParameterSet;
}

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

    using LinuxProcMgr = detail::LinuxProcMgr;
    std::unique_ptr<LinuxProcMgr> procInfo_;

    // Options
    unsigned numToSkip_;
    bool     truncateSummary_;
    bool     showMallocInfo_;
    bool     oncePerEventMode_;
    bool     moduleSummaryRequested_;

    using LinuxProcData = detail::LinuxProcData;
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
    template <unsigned SIZE, typename ... ARGS >
    using constrained_multimap = detail::constrained_multimap<SIZE,ARGS ...>;
    using dv_mapped_type = std::pair<art::EventID,double>;

    detail::constrained_multimap<3u,double,art::EventID>        sigEventMap_Vsize_;
    detail::constrained_multimap<5u,double,dv_mapped_type> sigEventMap_deltaVsize_;

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

#endif // art_Framework_Services_Optional_SimpleMemoryCheckLinux_h

// Local variables:
// mode: c++
// End:
