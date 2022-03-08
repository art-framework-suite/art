#ifndef art_Framework_Core_ProcessingLimits_h
#define art_Framework_Core_ProcessingLimits_h

#include "art/Framework/Core/InputSource.h"
#include "canvas/Persistency/Provenance/fwd.h"
#include "fhiclcpp/types/Atom.h"

#include <functional>

namespace art {
  class ProcessingLimits {
  public:
    struct Config {
      static constexpr char const*
      defaultMode()
      {
        return "RunsSubRunsAndEvents";
      }
      fhicl::Atom<int> maxEvents{fhicl::Name("maxEvents"), -1};
      fhicl::Atom<int> maxSubRuns{fhicl::Name("maxSubRuns"), -1};
      fhicl::Atom<int> reportFrequency{fhicl::Name("reportFrequency"), 1};
      fhicl::Atom<std::string> processingMode{fhicl::Name("processingMode"),
                                              defaultMode()};
    };

    ProcessingLimits(Config const& config,
                     std::function<input::ItemType()> nextItemType);

    input::ItemType nextItemType();
    InputSource::ProcessingMode processingMode() const noexcept;

    // Accessor for remaining number of events to be read.
    // -1 is used for unlimited.
    int remainingEvents() const noexcept;

    // Accessor for remaining number of subruns to be read.
    // -1 is used for unlimited.
    int remainingSubRuns() const noexcept;

    void update(EventID const& id);
    void update(SubRunID const& id);

  private:
    bool allowed_(input::ItemType) const noexcept;

    InputSource::ProcessingMode processingMode_{
      InputSource::RunsSubRunsAndEvents};
    int remainingEvents_;
    int remainingSubRuns_;
    int reportFrequency_;
    int numberOfEventsRead_{};
    std::function<input::ItemType()> nextItemType_;
  };
}

#endif /* art_Framework_Core_ProcessingLimits_h */

// Local Variables:
// mode: c++
// End:
