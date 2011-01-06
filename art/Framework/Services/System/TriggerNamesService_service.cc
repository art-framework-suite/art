#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Persistency/Common/TriggerResults.h"
#include "art/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSetRegistry.h"


using namespace cet;
using namespace fhicl;
using namespace std;


namespace art {
  namespace service {

    TriggerNamesService::TriggerNamesService(const ParameterSet& pset) {

      trigger_pset_ =
        pset.get<fhicl::ParameterSet>("trigger_paths");

      trignames_ = trigger_pset_.get<vector<string> >("trigger_paths");
      end_names_ = pset.get<vector<string> >("end_paths");

      ParameterSet defopts;
      ParameterSet opts =
        pset.get<fhicl::ParameterSet>("options", defopts);
      wantSummary_ =
        opts.get<bool>("wantSummary",false);

      process_name_ = pset.get<string>("process_name");

      loadPosMap(trigpos_,trignames_);
      loadPosMap(end_pos_,end_names_);

      const unsigned int n(trignames_.size());
      for(unsigned int i=0;i!=n;++i) {
        modulenames_.push_back(pset.get<vector<string> >(trignames_[i]));
      }
    }

    bool
    TriggerNamesService::getTrigPaths(TriggerResults const& triggerResults,
                                      Strings& trigPaths,
                                      bool& fromPSetRegistry) {

      // Get the parameter set containing the trigger names from the parameter set registry
      // using the ID from TriggerResults as the key used to find it.
      ParameterSet pset;
      if (ParameterSetRegistry::get(triggerResults.parameterSetID(), pset)) {

        trigPaths = pset.get<vector<string> >("trigger_paths",Strings());

        if (trigPaths.size() != triggerResults.size()) {
          throw art::Exception(art::errors::Unknown)
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

DEFINE_ART_SYSTEM_SERVICE(TriggerNamesService)

  }  // service
}  // art

