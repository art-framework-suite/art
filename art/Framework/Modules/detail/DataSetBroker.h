#ifndef art_Framework_Modules_detail_DataSetBroker_h
#define art_Framework_Modules_detail_DataSetBroker_h

#include "art/Framework/Modules/detail/DataSetSampler.h"
#include "art/Framework/Modules/detail/SamplingInputFile.h"
#include "canvas/Persistency/Provenance/ProcessHistoryID.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace art {
  namespace detail {

    using Products_t =
      std::map<BranchKey,
               std::map<std::string, std::vector<std::unique_ptr<EDProduct>>>>;

    class DataSetBroker {
    public:
      explicit DataSetBroker(fhicl::ParameterSet const& pset);

      std::map<BranchKey, BranchDescription> openInputFiles(
        BranchDescription const& sampledEventInfoDesc,
        ModuleDescription const& md,
        bool const readParameterSets,
        MasterProductRegistry& preg);

      bool canReadEvent();

      std::unique_ptr<SampledRunInfo> readAllRunProducts(
        Products_t& read_products);

      std::unique_ptr<SampledSubRunInfo> readAllSubRunProducts(
        Products_t& read_products);

      std::unique_ptr<EventPrincipal> readNextEvent(
        EventID const& id,
        ProcessConfigurations const& sampled_pcs,
        ProcessConfiguration const& current_pc);

      void countSummary() const;

    private:
      struct Config {
        std::string fileName;
        EventID firstEvent;
      };
      std::map<std::string, Config> configs_{};
      std::map<std::string, art::detail::SamplingInputFile> files_;
      std::unique_ptr<DataSetSampler> dataSetSampler_{nullptr};
      std::map<std::string, unsigned> counts_;
      unsigned totalCounts_{};
      cet::exempt_ptr<std::string const> currentDataset_{nullptr};
    };
  }
}

#endif /* art_Framework_Modules_detail_DataSetBroker_h */

// Local Variables:
// mode: c++
// End:
