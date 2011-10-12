#ifndef art_Framework_Core_EventSelector_h
#define art_Framework_Core_EventSelector_h

// ======================================================================
//
// EventSelector
//
// ======================================================================

#include "art/Persistency/Common/HLTPathStatus.h"
#include "art/Persistency/Common/TriggerResults.h"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"
#include <string>
#include <vector>

// ----------------------------------------------------------------------

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

  class EventSelector
  {
  public:

    typedef std::vector<std::string> Strings;

    EventSelector(Strings const& pathspecs,
                  Strings const& names);

    explicit
    EventSelector(Strings const& pathspecs);

    EventSelector(fhicl::ParameterSet const& pset,
                  Strings const& triggernames);

    bool wantAll() const { return accept_all_; }
    bool acceptEvent(TriggerResults const&);
    bool acceptEvent(unsigned char const*, int) const;

    std::shared_ptr<TriggerResults>
      maskTriggerResults(TriggerResults const& inputResults);
    static std::vector<std::string>
      getEventSelectionVString(fhicl::ParameterSet const& pset);

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

    typedef std::vector<BitInfo> Bits;

    bool accept_all_;
    Bits absolute_acceptors_;
    Bits conditional_acceptors_;
    Bits exception_acceptors_;
    std::vector<Bits> all_must_fail_;
    std::vector<Bits> all_must_fail_noex_;

    bool results_from_current_process_;
    bool psetID_initialized_;
    fhicl::ParameterSetID psetID_;

    Strings paths_;

    int nTriggerNames_;
    bool notStarPresent_;

    bool acceptTriggerPath(HLTPathStatus const&, BitInfo const&) const;

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
