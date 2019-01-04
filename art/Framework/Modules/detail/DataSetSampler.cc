#include "art/Framework/Modules/detail/DataSetSampler.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"

#include <iomanip>
#include <iterator>

art::detail::DataSetSampler::DataSetSampler(
  std::vector<std::string> const& datasetNames,
  std::vector<double> const& weights) noexcept(false)
  : datasetNames_{datasetNames}
  , weights_{weights}
  , dist_{cbegin(weights_), cend(weights_)}
{}

std::size_t
art::detail::DataSetSampler::index_for(std::string const& dataset) const
{
  auto const it = cet::find_in_all(datasetNames_, dataset);
  if (it == cend(datasetNames_)) {
    throw Exception{errors::LogicError}
      << "An index has been requested for dataset '" << dataset
      << "', which has\n"
      << "not been configured.  Please contact artists@fnal.gov for "
         "guidance.\n";
  }
  return static_cast<std::size_t>(std::distance(cbegin(datasetNames_), it));
}
