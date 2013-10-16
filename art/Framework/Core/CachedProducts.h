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

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Core/EventSelector.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Persistency/Common/TriggerResults.h"
//#include "cpp0x/utility"
#include <algorithm>
#include <string>
#include <vector>

namespace art {
namespace detail {

// Match events based on the trigger results from a given process name.
class ProcessAndEventSelector {
private:
  // Select events based on a process name.
  ProcessNameSelector processNameSelector_;
  // Select events based on criteria applied to trigger path results.
  EventSelector eventSelector_;
  // Fetched trigger path results for the given process name.
  art::Handle<art::TriggerResults> triggerResults_;
public:
  ProcessAndEventSelector(std::string const& process_name, EventSelector const& event_selector)
    : processNameSelector_(process_name), eventSelector_(event_selector), triggerResults_() {}
  //
  //  Trigger results.
  //
  void loadTriggerResults(Event const& e) { e.get(/*in=*/processNameSelector_, /*out=*/triggerResults_); }
  art::Handle<art::TriggerResults> triggerResults() const { return triggerResults_; }
  void clearTriggerResults() { triggerResults_ = art::Handle<art::TriggerResults>(); }
  //
  //  Event selection.
  //
  bool match() { return eventSelector_.acceptEvent(*triggerResults_); }
};

class PVSentry;

// Handle the SelectEvents configuration parameter of modules on the end path.
class CachedProducts {
  friend class PVSentry;
public: // Types
  //typedef art::Handle<art::TriggerResults> handle_t;
  //typedef std::vector<ProcessAndEventSelector> selectors_t;
  //typedef selectors_t::size_type size_type;
  //typedef std::pair<std::string, std::string> parsed_path_spec_t;
private: // Data Members
  std::vector<ProcessAndEventSelector> p_and_e_selectors_;
  bool loadDone_;
  unsigned long numberFound_;
public:
  CachedProducts() : p_and_e_selectors_(), loadDone_(false), numberFound_(0) {}
  void setupDefault(std::vector<std::string> const& trigger_names);
  void setup(std::vector<std::pair<std::string,std::string>> const& path_specs,
             std::vector<std::string> const& trigger_names,
             const std::string& process_name);
  bool wantEvent(Event const&);
  art::Handle<art::TriggerResults> getOneTriggerResults(Event const&) const;

private:
  void
  clearTriggerResults() {
    std::for_each(p_and_e_selectors_.begin(), p_and_e_selectors_.end(),
                  std::bind(&ProcessAndEventSelector::clearTriggerResults, std::placeholders::_1));
    loadDone_ = false;
    numberFound_ = 0;
  }

  void loadTriggerResults(Event const&);
};

class PVSentry {
private:
  CachedProducts& p_and_e_selectors_;
public:
  explicit PVSentry(CachedProducts& p_and_e_selectors)
    : p_and_e_selectors_(p_and_e_selectors) {}
  ~PVSentry() {
    p_and_e_selectors_.clearTriggerResults();
  }
  PVSentry(PVSentry const&) = delete;
  PVSentry& operator=(PVSentry const&) = delete;
};

} // namespace detail
} // namespace art

#endif /* art_Framework_Core_CachedProducts_h */

// Local Variables:
// mode: c++
// End:
