#ifndef art_Framework_Modules_detail_DataSetSampler_h
#define art_Framework_Modules_detail_DataSetSampler_h

#include <cassert>
#include <numeric>
#include <random>
#include <string>
#include <vector>

namespace fhicl {
  class ParameterSet;
}

namespace art {
  namespace detail {

    // We use a struct-of-arrays for format since it is better suited
    // for the facilities used here.
    class DataSetSampler {
    public:
      explicit DataSetSampler(fhicl::ParameterSet const& pset) noexcept(false);

      auto const&
      sample()
      {
        auto const index = dist_(engine_);
        assert(index < datasetNames_.size());
        return datasetNames_[index];
      }

      auto const&
      datasets() const noexcept
      {
        return datasetNames_;
      }

      auto
      weight(std::string const& dataset) const
      {
        return weights_[index_for(dataset)];
      }

      auto
      probability(std::string const& dataset) const
      {
        return dist_.probabilities()[index_for(dataset)];
      }

      auto const&
      fileName(std::string const& dataset) const
      {
        return fileNames_[index_for(dataset)];
      }

    private:
      std::size_t index_for(std::string const& dataset) const;

      std::vector<std::string> datasetNames_{};
      std::vector<double> weights_{};
      std::vector<std::string> fileNames_{};
      std::default_random_engine engine_{};
      std::discrete_distribution<unsigned> dist_{};
    };
  }
}

// Local variables:
// mode: c++
// End:

#endif /* art_Framework_Modules_detail_DataSetSampler_h */
