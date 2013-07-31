#ifndef art_Persistency_Common_TriggerResults_h
#define art_Persistency_Common_TriggerResults_h

// ======================================================================
//
// TriggerResults
//
// The trigger path results are maintained here as a sequence of entries,
// one per trigger path.  They are assigned in the order they appeared in
// the process-level pset.  (They are actually stored in the base class
// HLTGlobalStatus)
//
// The ParameterSetID can be used to get a ParameterSet from the registry
// of parameter sets.  This ParameterSet contains a vector<string> named
// "trigger_paths" that contains the trigger path names in the same order
// as the trigger path results stored here.
//
// ======================================================================

#include "art/Persistency/Common/HLTGlobalStatus.h"
#include "art/Persistency/Common/traits.h"
#include "fhiclcpp/ParameterSetID.h"

// ----------------------------------------------------------------------

namespace art {

  class TriggerResults
    : public HLTGlobalStatus
    , public DoNotRecordParents
  {
  private:
    // Parameter set id
    fhicl::ParameterSetID psetid_;

  public:
    // Default contructor
    TriggerResults()
      : HLTGlobalStatus()
      , psetid_()
    { }

    // Standard contructor
    TriggerResults(const HLTGlobalStatus& hlt,
                   const fhicl::ParameterSetID& psetid)
      : HLTGlobalStatus(hlt)
      , psetid_(psetid)
    { }

    // Get stored parameter set id
    const fhicl::ParameterSetID& parameterSetID() const { return psetid_; }

    // swap function
    void swap(TriggerResults& other) {
      this->HLTGlobalStatus::swap(other);
      psetid_.swap(other.psetid_);
    }

    // Copy assignment using swap.
    // We can't ref-qualify assignment because of GCC_XML.
    TriggerResults& operator=(TriggerResults const& rhs) {
      TriggerResults temp(rhs);
      this->swap(temp);
      return *this;
    }

  };  // TriggerResults

  // Free swap function
  inline
  void
  swap(TriggerResults& lhs, TriggerResults& rhs) {
    lhs.swap(rhs);
  }

}  // art

// ======================================================================

#endif /* art_Persistency_Common_TriggerResults_h */

// Local Variables:
// mode: c++
// End:
