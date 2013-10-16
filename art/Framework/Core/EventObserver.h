#ifndef art_Framework_Core_EventObserver_h
#define art_Framework_Core_EventObserver_h
// Common base class for module which do not modify events, such as
// OutputModule and EDAnalyzer.

#include "art/Framework/Core/CachedProducts.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"

#include <string>

namespace art {
  class MasterProductRegistry;
  class ModuleDescription;

  class EventObserver;
}

class art::EventObserver {
public:
  bool modifiesEvent() const { return false; }

  // FIXME: One could obviate the need for this trivial implementation
  // by putting some type logic in WorkerT.
  void registerProducts(MasterProductRegistry&, ModuleDescription const &) {}
  //
  // SelectEvents handling
  //
  std::string const& processName() const { return process_name_; }
  bool wantAllEvents() const { return wantAllEvents_; }
  fhicl::ParameterSetID selectorConfig() const { return selector_config_id_; }
  art::Handle<art::TriggerResults> getTriggerResults(Event const& e) const { return selectors_.getOneTriggerResults(e); }

protected:
  // True if no selectors configured.
  bool wantAllEvents_;
  // The process and event selectors, as specified by
  // the SelectEvents configuration parameter.
  detail::CachedProducts selectors_;
  explicit EventObserver(fhicl::ParameterSet const& pset);
  virtual ~EventObserver();

private:
  std::string process_name_;
  // ID of the ParameterSet that configured
  // the event selector subsystem.
  fhicl::ParameterSetID selector_config_id_;
};

#endif /* art_Framework_Core_EventObserver_h */

// Local Variables:
// mode: c++
// End:
