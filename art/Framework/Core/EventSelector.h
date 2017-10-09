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

  // possible return codes for the testSelectionOverlap
  // method defined below.
  namespace evtSel {

    enum OverlapResult {
      InvalidSelection = 0,
      NoOverlap = 1,
      PartialOverlap = 2,
      ExactMatch = 3
    };

  } // namespace evtSel

  class EventSelector {

  private: // TYPES
    struct BitInfo {

      BitInfo(unsigned const pos, bool const state)
        : pos_{pos}, accept_state_{state}
      {}

      unsigned int pos_{};

      bool accept_state_{false};
    };

  public: // MEMBER FUNCTIONS -- Special Member Functions
    EventSelector(std::vector<std::string> const& pathspecs,
                  std::vector<std::string> const& names);

    explicit EventSelector(std::vector<std::string> const& pathspecs);

    EventSelector(fhicl::ParameterSet const&,
                  std::vector<std::string> const& triggernames);

  public: // MEMBER FUNCTIONS
    bool wantAll() const;

    bool acceptEvent(TriggerResults const&);

    std::shared_ptr<TriggerResults> maskTriggerResults(
      TriggerResults const& inputResults);

  private: // MEMBER FUNCTIONS
    void init(std::vector<std::string> const& paths,
              std::vector<std::string> const& triggernames);

    bool acceptOneBit(std::vector<BitInfo> const&,
                      HLTGlobalStatus const&,
                      hlt::HLTState const& s = hlt::Ready) const;

    bool acceptAllBits(std::vector<BitInfo> const&,
                       HLTGlobalStatus const&) const;

    bool containsExceptions(HLTGlobalStatus const&) const;

    bool selectionDecision(HLTGlobalStatus const&) const;

  private: // MEMBER FUNCTIONS -- Static interface
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

  private: // MEMBER DATA
    bool accept_all_{false};

    std::vector<BitInfo> absolute_acceptors_{};

    std::vector<BitInfo> conditional_acceptors_{};

    std::vector<BitInfo> exception_acceptors_{};

    std::vector<std::vector<BitInfo>> all_must_fail_{};

    std::vector<std::vector<BitInfo>> all_must_fail_noex_{};

    bool results_from_current_process_{true};

    bool psetID_initialized_{false};

    fhicl::ParameterSetID psetID_{};

    std::vector<std::string> paths_{};

    int nTriggerNames_{0};

    bool notStarPresent_{false};
  };

} // namespace art

#endif /* art_Framework_Core_EventSelector_h */

// Local Variables:
// mode: c++
// End:
