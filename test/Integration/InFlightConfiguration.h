#ifndef test_Integration_InFlightConfiguration_h
#define test_Integration_InFlightConfiguration_h

#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/System/PathSelection.h"
#include "art/Framework/Services/UserInteraction/UserInteraction.h"
#include "fhiclcpp/ParameterSet.h"

#include <vector>

namespace arttest {
  class InFlightConfiguration;
}

class arttest::InFlightConfiguration : public ui::UserInteraction {
public:
  InFlightConfiguration(fhicl::ParameterSet const & ps,
                        art::ActivityRegistry & areg);

  void moduleList(std::vector<ModuleInfo> const & infos) override;
  void pickModule() override;
  UserInteraction::NextStep nextAction() override;

private:
  std::vector<ModuleInfo> moduleInfos_;
  // Note user implementations would cache by value, not pointer.
  std::unique_ptr<art::ServiceHandle<art::PathSelection> > pathSelectionService_;

  size_t callCount_;
};

DECLARE_ART_SERVICE(arttest::InFlightConfiguration, LEGACY)
#endif /* test_Integration_InFlightConfiguration_h */

// Local Variables:
// mode: c++
// End:
