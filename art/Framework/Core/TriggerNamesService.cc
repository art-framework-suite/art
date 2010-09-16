#include "art/Framework/Core/TriggerNamesService.h"

#include "art/ParameterSet/Registry.h"
#include "art/Persistency/Common/TriggerResults.h"
#include "art/Utilities/Algorithms.h"
#include "art/Utilities/EDMException.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/ThreadSafeRegistry.h"

using fhicl::ParameterSet;


namespace edm {
  namespace service {

    TriggerNamesService::TriggerNamesService(const ParameterSet& pset) {

      trigger_pset_ =
        pset.getPSet("@trigger_paths");

      trignames_ = trigger_pset_.getVString("@trigger_paths");
      end_names_ = pset.getVString("@end_paths");

      ParameterSet defopts;
      ParameterSet opts =
        pset.getPSet("options", defopts);
      wantSummary_ =
        opts.getBool("wantSummary",false);

      process_name_ = pset.getString("@process_name");

      loadPosMap(trigpos_,trignames_);
      loadPosMap(end_pos_,end_names_);

      const unsigned int n(trignames_.size());
      for(unsigned int i=0;i!=n;++i) {
        modulenames_.push_back(pset.getVString(trignames_[i]));
      }
    }

    bool
    TriggerNamesService::getTrigPaths(TriggerResults const& triggerResults,
                                      Strings& trigPaths,
                                      bool& fromPSetRegistry) {

      // Get the parameter set containing the trigger names from the parameter set registry
      // using the ID from TriggerResults as the key used to find it.
      ParameterSet pset;
      pset::Registry* psetRegistry = pset::Registry::instance();
      if (psetRegistry->getMapped(triggerResults.parameterSetID(),
                                  pset)) {

        trigPaths = pset.getVString("@trigger_paths",Strings());

        if (trigPaths.size() != triggerResults.size()) {
          throw edm::Exception(edm::errors::Unknown)
            << "TriggerNamesService::getTrigPaths, Trigger names vector and\n"
               "TriggerResults are different sizes.  This should be impossible,\n"
               "please send information to reproduce this problem to\n"
               "the edm developers.\n";
        }

        fromPSetRegistry = true;
        return true;
      }

      fromPSetRegistry = false;

      // In older versions of the code the the trigger names were stored
      // inside of the TriggerResults object.  This will provide backward
      // compatibility.
      if (triggerResults.size() == triggerResults.getTriggerNames().size()) {
        trigPaths = triggerResults.getTriggerNames();
        return true;
      }

      return false;
    }

    bool
    TriggerNamesService::getTrigPaths(TriggerResults const& triggerResults,
                                      Strings& trigPaths) {
      bool dummy;
      return getTrigPaths(triggerResults, trigPaths, dummy);
    }

  }  // namespace service
}  // namespace edm
