#ifndef art_Framework_Core_EventObserverBase_h
#define art_Framework_Core_EventObserverBase_h
// vim: set sw=2 expandtab :

// Common base class for module which do not modify events, such as
// OutputModule and EDAnalyzer.

#include "art/Framework/Core/CachedProducts.h"
#include "art/Framework/Core/ModuleBase.h"
#include "art/Framework/Core/ModuleType.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"
#include "fhiclcpp/types/OptionalTable.h"
#include "fhiclcpp/types/Sequence.h"
#include "hep_concurrency/SerialTaskQueueChain.h"

#include <set>
#include <string>

namespace art {

  class ModuleDescription;

  class EventObserverBase : public ModuleBase {

  protected: // TYPES
    struct EOConfig {

      fhicl::Sequence<std::string> selectEvents{
        fhicl::Name("SelectEvents"),
        fhicl::Comment("The following parameter is a user-provided list\n"
                       "of filter paths. The default list is empty."),
        std::vector<std::string>{}};
    };

  public: // MEMBER FUNCTIONS -- Special Member Functions
    // FIXME: This class need not be virtual.

    virtual ~EventObserverBase();

    EventObserverBase(EventObserverBase const&) = delete;

    EventObserverBase(EventObserverBase&&) = delete;

    EventObserverBase& operator=(EventObserverBase const&) = delete;

    EventObserverBase& operator=(EventObserverBase&&) = delete;

  protected: // MEMBER FUNCTIONS -- Special Member Functions
    explicit EventObserverBase(fhicl::ParameterSet const& config);

    explicit EventObserverBase(std::vector<std::string> const& paths,
                               fhicl::ParameterSet const& config);

  public:
    // FIXME: One could obviate the need for this trivial implementation
    // by putting some type logic in WorkerT.
    void registerProducts(ProductDescriptions&, ModuleDescription const&);

    std::string const& processName() const;

    bool wantAllEvents() const;

    bool wantEvent(Event const& e);

    fhicl::ParameterSetID selectorConfig() const;

    Handle<TriggerResults> getTriggerResults(Event const& e) const;

  protected:
    detail::CachedProducts& cachedProducts();

  private: // MEMBER FUNCTIONS -- Implementation details.
    void init_(std::vector<std::string> const& paths);

  private: // MEMBER DATA
    // True if no selectors configured.
    bool wantAllEvents_{false};

    // The process and event selectors, as specified by the SelectEvents
    // configuration parameter.
    detail::CachedProducts selectors_{};

    std::string process_name_{};

    // ID of the ParameterSet that configured the event selector
    // subsystem.
    fhicl::ParameterSetID selector_config_id_{};
  };

} // namespace art

#endif /* art_Framework_Core_EventObserverBase_h */

// Local Variables:
// mode: c++
// End:
