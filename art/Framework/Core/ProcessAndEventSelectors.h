#ifndef art_Framework_Core_ProcessAndEventSelectors_h
#define art_Framework_Core_ProcessAndEventSelectors_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/EventSelector.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Common/TriggerResults.h"

#include <string>
#include <utility>
#include <vector>

namespace art::detail {
  // Match events based on the trigger results from a given process name.
  class ProcessAndEventSelector {
  public:
    explicit ProcessAndEventSelector(std::string const& process,
                                     EventSelector const& es);

    art::Handle<art::TriggerResults> triggerResults(Event const& e) const;
    bool match(Event const& e) const;

  private:
    ProcessNameSelector processNameSelector_;
    mutable EventSelector eventSelector_;
  };

  class ProcessAndEventSelectors {
  public:
    ProcessAndEventSelectors(
      std::vector<std::pair<std::string, std::string>> const& path_specs,
      std::vector<std::string> const& trigger_names,
      std::string const& process_name);

    bool matchEvent(Event const& e) const;
    art::Handle<art::TriggerResults> getOneTriggerResults(Event const&) const;

  private:
    std::vector<ProcessAndEventSelector> sel_{};
  };

} // namespace art::detail

#endif /* art_Framework_Core_ProcessAndEventSelectors_h */

// Local Variables:
// mode: c++
// End:
