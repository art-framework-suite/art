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

    bool wantAll() const;
    bool acceptEvent(TriggerResults const&);
    std::shared_ptr<TriggerResults> maskTriggerResults(
      TriggerResults const& inputResults);

  private:
    struct BitInfo {
      BitInfo(unsigned const pos, bool const state)
        : pos_{pos}, accept_state_{state}
      {}
      unsigned int pos_{};
      bool accept_state_{false};
    };

    void init(std::vector<std::string> const& paths,
              std::vector<std::string> const& triggernames);
    bool acceptOneBit(std::vector<BitInfo> const&,
                      HLTGlobalStatus const&,
                      hlt::HLTState const& s = hlt::Ready) const;
    bool acceptAllBits(std::vector<BitInfo> const&,
                       HLTGlobalStatus const&) const;
    bool containsExceptions(HLTGlobalStatus const&) const;
    bool selectionDecision(HLTGlobalStatus const&) const;

    static std::vector<std::vector<std::string>::const_iterator>
    matching_triggers(std::vector<std::string> const& trigs,
                      std::string const&);
    static bool identical(std::vector<bool> const&, std::vector<bool> const&);
    static bool identical(EventSelector const&,
                          EventSelector const&,
                          unsigned int);
    static std::vector<bool> expandDecisionList(std::vector<BitInfo> const&,
                                                bool PassOrFail,
                                                unsigned int);
    static bool overlapping(std::vector<bool> const&, std::vector<bool> const&);
    static bool subset(std::vector<bool> const&, std::vector<bool> const&);
    static std::vector<bool> combine(std::vector<bool> const&,
                                     std::vector<bool> const&);

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
    int nTriggerNames_{0};
    bool notStarPresent_{false};
  };

} // namespace art

#endif /* art_Framework_Core_EventSelector_h */

// Local Variables:
// mode: c++
// End:
