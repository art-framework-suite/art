#ifndef art_Framework_Core_CachedProducts_h
#define art_Framework_Core_CachedProducts_h

// -------------------------------------------------------------------
//
// CachedProducts: This class is used by OutputModule to interact with
// the TriggerResults objects upon which the decision to write out an
// event is made.
//
//
// -------------------------------------------------------------------

#include "art/Framework/Core/EventSelector.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Common/TriggerResults.h"

#include <string>
#include <vector>

namespace art {
  namespace detail {

    // Match events based on the trigger results from a given process name.
    class ProcessAndEventSelector {
    public:
      explicit ProcessAndEventSelector(std::string const& process_name,
                                       EventSelector const& event_selector)
        : processNameSelector_{process_name}, eventSelector_{event_selector}
      {}

      // Trigger results.
      void
      loadTriggerResults(Event const& e)
      {
        e.get(/*in=*/processNameSelector_, /*out=*/triggerResults_);
      }
      art::Handle<art::TriggerResults>
      triggerResults() const
      {
        return triggerResults_;
      }
      void
      clearTriggerResults()
      {
        triggerResults_.clear();
      }

      // Event selection.
      bool
      match()
      {
        return eventSelector_.acceptEvent(*triggerResults_);
      }

    private:
      // Select events based on a process name.
      ProcessNameSelector processNameSelector_;
      // Select events based on criteria applied to trigger path results.
      EventSelector eventSelector_;
      // Fetched trigger path results for the given process name.
      art::Handle<art::TriggerResults> triggerResults_{}; // invalid
    };

    class PVSentry;

    // Handle the SelectEvents configuration parameter of modules on the end
    // path.
    class CachedProducts {
    public:
      void setupDefault(std::vector<std::string> const& trigger_names);
      void setup(
        std::vector<std::pair<std::string, std::string>> const& path_specs,
        std::vector<std::string> const& trigger_names,
        std::string const& process_name);
      bool wantEvent(Event const&);
      art::Handle<art::TriggerResults> getOneTriggerResults(Event const&) const;

    private:
      void clearTriggerResults();
      void loadTriggerResults(Event const&);

      friend class PVSentry;
      std::vector<ProcessAndEventSelector> p_and_e_selectors_{};
      bool loadDone_{false};
      unsigned long numberFound_{};
    };

    class PVSentry {
    public:
      explicit PVSentry(CachedProducts& p_and_e_selectors)
        : p_and_e_selectors_(p_and_e_selectors)
      {}

      ~PVSentry() noexcept(false) { p_and_e_selectors_.clearTriggerResults(); }

      PVSentry(PVSentry const&) = delete;
      PVSentry& operator=(PVSentry const&) = delete;

    private:
      CachedProducts& p_and_e_selectors_;
    };

  } // namespace detail
} // namespace art

#endif /* art_Framework_Core_CachedProducts_h */

// Local Variables:
// mode: c++
// End:
