#ifndef art_Framework_Core_ProcessAndEventSelectors_h
#define art_Framework_Core_ProcessAndEventSelectors_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/EventSelector.h"
#include "art/Framework/Principal/Event.h"
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
    ~ProcessAndEventSelector();
    explicit ProcessAndEventSelector(std::string const& process,
                                     EventSelector const&);

    void loadTriggerResults(Event const&);
    art::Handle<art::TriggerResults> triggerResults() const;
    bool match();
    void clearTriggerResults();

  private:
    ProcessNameSelector processNameSelector_;
    EventSelector eventSelector_;
    art::Handle<art::TriggerResults> triggerResults_{};
  };

  // Handle the SelectEvents configuration parameter
  // of modules on the end path.
  class ProcessAndEventSelectors {
  public:
    ~ProcessAndEventSelectors();
    ProcessAndEventSelectors();
    ProcessAndEventSelectors(ProcessAndEventSelectors const&) = delete;
    ProcessAndEventSelectors(ProcessAndEventSelectors&&) = delete;
    ProcessAndEventSelectors& operator=(ProcessAndEventSelectors const&) =
      delete;
    ProcessAndEventSelectors& operator=(ProcessAndEventSelectors&&) = delete;

    void setupDefault(std::vector<std::string> const& trigger_names);
    void setup(
      std::vector<std::pair<std::string, std::string>> const& path_specs,
      std::vector<std::string> const& trigger_names,
      std::string const& process_name);
    bool wantEvent(Event const&);
    art::Handle<art::TriggerResults> getOneTriggerResults(Event const&);
    void clearTriggerResults();

  private:
    std::vector<ProcessAndEventSelector> sel_{};
    bool loadDone_{false};
    unsigned long numberFound_{};
  };

  class PVSentry {
  public:
    ~PVSentry();
    explicit PVSentry(ProcessAndEventSelectors&);
    PVSentry(PVSentry const&) = delete;
    PVSentry(PVSentry&&) = delete;
    PVSentry& operator=(PVSentry&&) = delete;

  private:
    ProcessAndEventSelectors& sel_;
  };
} // namespace art::detail

#endif /* art_Framework_Core_ProcessAndEventSelectors_h */

// Local Variables:
// mode: c++
// End:
