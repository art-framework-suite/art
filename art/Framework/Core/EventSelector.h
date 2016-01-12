#ifndef art_Framework_Core_EventSelector_h
#define art_Framework_Core_EventSelector_h

// ======================================================================
//
// EventSelector
//
// ======================================================================

#include "art/Persistency/Common/HLTPathStatus.h"
#include "art/Persistency/Common/TriggerResults.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"

#include <memory>
#include <string>
#include <vector>

// ----------------------------------------------------------------------

#define ACCEPT_EVENT_MSG                                                \
  "\n\n"                                                                \
  "art warning: This function has been deprecated.\n"                   \
  "             Please contact artists@fnal.gov for assistance.\n"      \
  "\n"

namespace art {

  // possible return codes for the testSelectionOverlap
  // method defined below.
  namespace evtSel
  {
    enum OverlapResult {InvalidSelection = 0,
                        NoOverlap = 1,
                        PartialOverlap = 2,
                        ExactMatch = 3};
  }  // evtSel

  class EventSelector {
  public:

    using Strings = std::vector<std::string>;

    EventSelector(Strings const& pathspecs,
                  Strings const& names);

    explicit
    EventSelector(Strings const& pathspecs);

    EventSelector(fhicl::ParameterSet const& pset,
                  Strings const& triggernames);

    bool wantAll() const { return accept_all_; }
    bool acceptEvent(TriggerResults const&);
    [[deprecated(ACCEPT_EVENT_MSG)]] bool acceptEvent(unsigned char const*, int) const;

    std::shared_ptr<TriggerResults>
    maskTriggerResults(TriggerResults const& inputResults);

  private:

    void init(Strings const& paths,
              Strings const& triggernames);

    struct BitInfo
    {
      BitInfo(unsigned int pos, bool state):pos_(pos),accept_state_(state) { }
      BitInfo():pos_(),accept_state_() { }

      unsigned int pos_;
      bool accept_state_;
    };

    using Bits = std::vector<BitInfo>;

    bool accept_all_ {false};
    Bits absolute_acceptors_ {};
    Bits conditional_acceptors_ {};
    Bits exception_acceptors_ {};
    std::vector<Bits> all_must_fail_ {};
    std::vector<Bits> all_must_fail_noex_ {};

    bool results_from_current_process_ {true};
    bool psetID_initialized_ {false};
    fhicl::ParameterSetID psetID_ {};

    Strings paths_ {};

    int nTriggerNames_ {0};
    bool notStarPresent_ {false};

    bool acceptOneBit (Bits const & b,
                       HLTGlobalStatus const & tr,
                       hlt::HLTState const & s = hlt::Ready) const;
    bool acceptAllBits (Bits const & b,
                        HLTGlobalStatus const & tr) const;

    bool containsExceptions(HLTGlobalStatus const & tr) const;

    bool selectionDecision(HLTGlobalStatus const & tr) const;

    static std::vector< Strings::const_iterator >
    matching_triggers(Strings const& trigs, std::string const& s);

    static bool identical (std::vector<bool> const & a,
                           std::vector<bool> const & b);
    static bool identical (EventSelector const & a,
                           EventSelector const & b,
                           unsigned int N);
    static std::vector<bool> expandDecisionList (
                Bits const & b,
                bool PassOrFail,
                unsigned int n);
    static bool overlapping ( std::vector<bool> const& a,
                              std::vector<bool> const& b );
    static bool subset  ( std::vector<bool> const& a,
                          std::vector<bool> const& b );
    static std::vector<bool> combine ( std::vector<bool> const& a,
                                       std::vector<bool> const& b );
  };  // EventSelector

}  // art

// ======================================================================

#endif /* art_Framework_Core_EventSelector_h */

// Local Variables:
// mode: c++
// End:
