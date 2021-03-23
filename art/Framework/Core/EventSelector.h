#ifndef art_Framework_Core_EventSelector_h
#define art_Framework_Core_EventSelector_h
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Common/HLTPathStatus.h"
#include "canvas/Persistency/Common/TriggerResults.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"

#include <memory>
#include <string>
#include <vector>

namespace art {
  class EventSelector {
  public:
    EventSelector(std::vector<std::string> const& pathspecs,
                  std::vector<std::string> const& trigger_path_names);
    explicit EventSelector(std::vector<std::string> const& pathspecs);

    bool acceptEvent(TriggerResults const&);

  private:
    struct BitInfo {
      unsigned int pos{};
      bool accept_state{false};
    };

    void init(std::vector<std::string> const& paths,
              std::vector<std::string> const& triggernames);
    bool acceptOneBit(std::vector<BitInfo> const&,
                      HLTGlobalStatus const&,
                      hlt::HLTState const& s = hlt::Ready) const;
    bool acceptAllBits(std::vector<BitInfo> const&,
                       HLTGlobalStatus const&) const;
    bool selectionDecision(HLTGlobalStatus const&) const;

    bool accept_all_{false};
    std::vector<BitInfo> absolute_acceptors_{};
    std::vector<BitInfo> conditional_acceptors_{};
    std::vector<BitInfo> exception_acceptors_{};
    std::vector<std::vector<BitInfo>> all_must_fail_{};
    std::vector<std::vector<BitInfo>> all_must_fail_noex_{};
    bool force_results_from_current_process_{true};
    bool psetID_initialized_{false};
    fhicl::ParameterSetID psetID_{};
    std::vector<std::string> pathspecs_{};
  };

} // namespace art

#endif /* art_Framework_Core_EventSelector_h */

// Local Variables:
// mode: c++
// End:
