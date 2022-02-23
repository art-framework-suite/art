#ifndef art_Framework_Core_EventSelector_h
#define art_Framework_Core_EventSelector_h
// vim: set sw=2 expandtab :

#include "art/Utilities/PerScheduleContainer.h"
#include "canvas/Persistency/Common/fwd.h"
#include "fhiclcpp/ParameterSetID.h"

#include <memory>
#include <string>
#include <vector>

namespace art {
  class EventSelector {
  public:
    explicit EventSelector(std::vector<std::string> const& pathspecs);
    EventSelector(EventSelector const&);
    EventSelector(EventSelector&&);
    ~EventSelector();

    bool acceptEvent(ScheduleID id, TriggerResults const& tr) const;

    struct BitInfo {
      unsigned int pos{};
      bool accept_state{false};
    };

  private:
    std::vector<std::string> const path_specs_;
    bool const accept_all_;
    struct ScheduleData {
      fhicl::ParameterSetID psetID{};
      std::vector<BitInfo> absolute_acceptors;
      std::vector<BitInfo> conditional_acceptors;
      std::vector<BitInfo> exception_acceptors;
      std::vector<std::vector<BitInfo>> all_must_fail;
      std::vector<std::vector<BitInfo>> all_must_fail_noex;
    };
    PerScheduleContainer<ScheduleData> mutable acceptors_;

    ScheduleData data_for(TriggerResults const& tr) const;
    bool selectionDecision(ScheduleData const& data,
                           HLTGlobalStatus const&) const;
  };

} // namespace art

#endif /* art_Framework_Core_EventSelector_h */

// Local Variables:
// mode: c++
// End:
