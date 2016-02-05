#ifndef art_Framework_Services_Optional_TimeTracker_h
#define art_Framework_Services_Optional_TimeTracker_h

// ======================================================================
//
// TimeTracker
//
// ======================================================================

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "art/Ntuple/Ntuple.h"
#include "art/Ntuple/sqlite_DBmanager.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Name.h"
#include "fhiclcpp/types/Table.h"
#include "tbb/tick_count.h"

#include <string>

namespace art {

  class TimeTracker {
  public:

    struct Config {
      fhicl::Atom<bool> printSummary { fhicl::Name("printSummary"), true };

      struct DBoutput {
        fhicl::Atom<std::string> filename { fhicl::Name("filename"), "" };
        fhicl::Atom<bool> overwrite { fhicl::Name("overwrite"), false };
      };
      fhicl::Table<DBoutput> dbOutput { fhicl::Name("dbOutput") };
    };

    using Parameters = ServiceTable<Config>;
    TimeTracker(ServiceTable<Config> const &, ActivityRegistry&);

  private:

    void prePathProcessing(std::string const&);

    void postEndJob();

    void preEventProcessing(Event const&);
    void postEventProcessing(Event const&);

    void preModule(ModuleDescription const&);
    void postModule(ModuleDescription const&);

    std::string pathname_;
    EventID eventId_;
    tbb::tick_count eventStart_;
    tbb::tick_count moduleStart_;

    bool printSummary_;
    sqlite::DBmanager dbMgr_;
    bool overwriteContents_;

    template<unsigned SIZE>
    using name_array = ntuple::name_array<SIZE>;
    name_array<7u> timeReportTuple_;
    name_array<4u> timeEventTuple_;
    name_array<5u> timeModuleTuple_;

    using timeReport_t = ntuple::Ntuple<std::string,double,double,double,double,double,uint32_t>;
    using timeEvent_t  = ntuple::Ntuple<uint32_t,uint32_t,uint32_t,double>;
    using timeModule_t = ntuple::Ntuple<uint32_t,uint32_t,uint32_t,std::string,double>;

    timeReport_t timeReportTable_;
    timeEvent_t  timeEventTable_;
    timeModule_t timeModuleTable_;

  };  // TimeTracker

} // namespace art

#endif /* art_Framework_Services_Optional_TimeTracker_h */

// Local variables:
// mode: c++
// End:
