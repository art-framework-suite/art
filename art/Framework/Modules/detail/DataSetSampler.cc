#include "art/Framework/Modules/detail/DataSetSampler.h"
#include "art/Framework/Modules/detail/event_start.h"
#include "art/Utilities/bold_fontify.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Sequence.h"
#include "fhiclcpp/types/Table.h"

#include <iterator>

using namespace fhicl;

namespace {
  struct DataSetConfig {
    Sequence<std::string> fileNames{Name{"fileNames"}};
    Atom<double> weight{Name{"weight"}};
    Atom<std::string> skipToEvent{
      Name{"skipToEvent"},
      Comment{"The 'skipToEvent' parameter specifies the first event that\n"
              "should be used when sampling from a specific input file.  The\n"
              "correct specification is via the triplet \"run:subrun:event\".\n"
              "For example, to begin sampling at run 3, subrun 2, event 39\n"
              "from this dataset, the user should specify\n\n"
              "  skipToEvent: \"3:2:39\"\n\n"
              "Specifying the empty string (the default) is equivalent to\n"
              "beginning at the first event of the file.  All other\n"
              "specifications are ill-formed and will result in an exception\n"
              "throw at the beginning of the job."},
      ""};
  };

  auto
  make_exception_for(std::string const& dataset)
  {
    return art::Exception{art::errors::Configuration}
           << "\nModule label: " << art::detail::bold_fontify("source")
           << "\nmodule_type : " << art::detail::bold_fontify("SamplingInput")
           << "\ndataset     : " << art::detail::bold_fontify(dataset);
  }

  auto
  first_event_id(std::string const& firstEvent)
  {
    using namespace art;
    if (firstEvent.empty()) {
      // Returning an invalid event as the first event is supported
      // because invalid events always compare less than valid events.
      return EventID::invalidEvent();
    }
    RunNumber_t r; SubRunNumber_t sr; EventNumber_t e;
    std::tie(r, sr, e) = art::detail::event_start(firstEvent);
    return EventID{r, sr, e};
  }
}

art::detail::DataSetSampler::DataSetSampler(ParameterSet const& pset) noexcept(
  false)
{
  auto const dataset_names = pset.get_pset_names();
  if (dataset_names.empty()) {
    throw Exception{
      errors::Configuration,
      "An error occurred while processing dataset configurations.\n"}
      << "No datasets were configured for the SamplingInput source.\n"
         "At least one must be specified.\n";
  }
  auto const ndatasets = dataset_names.size();
  datasetNames_.reserve(ndatasets);
  weights_.reserve(ndatasets);
  fileNames_.reserve(ndatasets);
  for (auto const& dataset : dataset_names) {
    auto const& dataset_pset = pset.get<ParameterSet>(dataset);
    try {
      Table<DataSetConfig> table{dataset_pset};
      datasetNames_.push_back(dataset);
      weights_.push_back(table().weight());
      auto const filenames = table().fileNames();
      if (filenames.size() != 1ull) {
        throw make_exception_for(dataset)
          << "\n\n"
             "The 'fileNames' sequence must contain 1 and only 1 filename.\n"
             "The ability to specify multiple file names may be possible in "
             "the future.\n"
             "Please contact artists@fnal.gov for guidance.";
      }
      fileNames_.push_back(filenames.front());
      auto const firstEvent = table().skipToEvent();
      firstEvents_.push_back(first_event_id(firstEvent));
    }
    catch (fhicl::detail::validationException const& e) {
      throw make_exception_for(dataset) << "\n\n" << e.what();
    }
    catch (art::Exception const& e) {
      throw make_exception_for(dataset) << "\n\n" << e.what();
    }
  }
  dist_ = decltype(dist_){cbegin(weights_), cend(weights_)};
}

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
