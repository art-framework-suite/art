#ifndef art_Framework_Core_Observer_h
#define art_Framework_Core_Observer_h
// vim: set sw=2 expandtab :

// Common base class for module which do not modify events, such as
// OutputModule and EDAnalyzer.

#include "art/Framework/Core/ModuleBase.h"
#include "art/Framework/Core/ProcessAndEventSelectors.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"
#include "fhiclcpp/types/OptionalTable.h"
#include "fhiclcpp/types/Sequence.h"

#include <set>
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

  protected:
    struct EOConfig {
      fhicl::Sequence<std::string> selectEvents{
        fhicl::Name("SelectEvents"),
        fhicl::Comment("The following parameter is a user-provided list\n"
                       "of filter paths. The default list is empty."),
        std::vector<std::string>{}};
    };

    explicit Observer(fhicl::ParameterSet const& config);
    explicit Observer(std::vector<std::string> const& paths,
                      fhicl::ParameterSet const& config);

  public:
    // FIXME: One could obviate the need for this trivial implementation
    // by putting some type logic in WorkerT.
    void registerProducts(ProductDescriptions&, ModuleDescription const&);
    void fillDescriptions(ModuleDescription const&);
    std::string const& processName() const;
    bool wantAllEvents() const;
    bool wantEvent(Event const& e);
    fhicl::ParameterSetID selectorConfig() const;
    Handle<TriggerResults> getTriggerResults(Event const& e) const;

  protected:
    detail::ProcessAndEventSelectors& processAndEventSelectors();

  private:
    void init_(std::vector<std::string> const& paths);

    // True if no selectors configured.
    bool wantAllEvents_{false};
    // The process and event selectors, as specified by the SelectEvents
    // configuration parameter.
    mutable detail::ProcessAndEventSelectors selectors_{};
    std::string process_name_{};
    // ID of the ParameterSet that configured the event selector
    // subsystem.
    fhicl::ParameterSetID selector_config_id_{};
  };

} // namespace art

#endif /* art_Framework_Core_Observer_h */

// Local Variables:
// mode: c++
// End:
