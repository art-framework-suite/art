#ifndef art_test_Integration_Reconfigurable_h
#define art_test_Integration_Reconfigurable_h
// ======================================================================
//
// ======================================================================

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "fhiclcpp/fwd.h"
// ----------------------------------------------------------------------

class Reconfigurable
{
public:
  Reconfigurable(fhicl::ParameterSet const& cfg, art::ActivityRegistry& ar);

  void reconfigure(fhicl::ParameterSet const& cfg);
  int get_debug_level() { return debug_level_; }

  bool postBeginJobCalled() const { return postBeginJobCalled_; }

private:
  void postBeginJob();

  int debug_level_;
  int other_value_;

  bool postBeginJobCalled_;

};

DECLARE_ART_SERVICE(Reconfigurable, LEGACY)
#endif /* art_test_Integration_Reconfigurable_h */



// Local Variables:
// mode: c++
// End:
