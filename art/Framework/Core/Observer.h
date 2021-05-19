#ifndef art_Framework_Core_Observer_h
#define art_Framework_Core_Observer_h
// vim: set sw=2 expandtab :

// Common base class for module which do not modify events, such as
// OutputModule and EDAnalyzer.

#include "art/Framework/Core/ModuleBase.h"
#include "art/Framework/Core/ProcessAndEventSelectors.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "fhiclcpp/ParameterSetID.h"
#include "fhiclcpp/fwd.h"
#include "fhiclcpp/types/Sequence.h"

#include <string>

namespace art {
  class ModuleDescription;

  class Observer : public ModuleBase {
  public:
    ~Observer() noexcept;
    Observer(Observer const&) = delete;
    Observer(Observer&&) = delete;
    Observer& operator=(Observer const&) = delete;
    Observer& operator=(Observer&&) = delete;

    // FIXME: One could obviate the need for this trivial implementation
    // by putting some type logic in WorkerT.
    void registerProducts(ProductDescriptions&, ModuleDescription const&);
    void fillDescriptions(ModuleDescription const&);
    fhicl::ParameterSetID selectorConfig() const;

  protected:
    std::string const& processName() const;
    bool
    wantAllEvents() const noexcept
    {
      return wantAllEvents_;
    }
    bool wantEvent(ScheduleID id, Event const& e) const;
    Handle<TriggerResults> getTriggerResults(Event const& e) const;

    struct EOConfig {
      fhicl::Sequence<std::string> selectEvents{
        fhicl::Name("SelectEvents"),
        fhicl::Comment{
          "Events are selected based on the trigger-path entries provided in\n"
          "the 'SelectEvents' and 'RejectEvents' parameters.  For example, a\n"
          "configuration of\n\n"
          "  SelectEvents: [A, B]\n"
          "  RejectEvents: [C, D]\n\n"
          "would accept events that satisfy trigger-path criteria A or B and\n"
          "fail criteria C or D.  In other words, the event is accepted if "
          "the\n"
          "following Boolean expression evaluates to true:\n\n"
          "  (A || B) and not (C || D)\n\n"
          "For the majority of cases, a trigger-path criterion may be:\n\n"
          "  1. A trigger-path name in the current process (e.g. tp)\n"
          "  2. A negated trigger-path name in the current process (e.g. "
          "\"!tp\")\n"
          "  3. A trigger-path name from a previous process (e.g. "
          "\"previousProcess:tp\")\n\n"
          "More complicated expressions are allowed--see\n"
          "  "
          "https://cdcvs.fnal.gov/redmine/projects/art/wiki/"
          "Filtering_events\n\n"
          "The default 'SelectEvents' and `RejectEvents` lists are empty,\n"
          "which is equivalent to selecting all events."},
        std::vector<std::string>{}};
      fhicl::Sequence<std::string> rejectEvents{fhicl::Name("RejectEvents"),
                                                std::vector<std::string>{}};
    };

    explicit Observer(fhicl::ParameterSet const& config);
    explicit Observer(std::vector<std::string> const& select_paths,
                      std::vector<std::string> const& reject_paths,
                      fhicl::ParameterSet const& config);

  private:
    // True if no selectors configured.
    bool wantAllEvents_;
    std::string process_name_;
    // The process and event selectors and rejectors, as specified by
    // the SelectEvents and RejectEvents configuration parameters.
    std::optional<detail::ProcessAndEventSelectors> selectors_;
    std::optional<detail::ProcessAndEventSelectors> rejectors_;
    // ID of the ParameterSet that configured the event selector
    // subsystem.
    fhicl::ParameterSetID selector_config_id_;
  };

} // namespace art

#endif /* art_Framework_Core_Observer_h */

// Local Variables:
// mode: c++
// End:
