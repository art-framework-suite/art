#ifndef test_Integration_Reconfigurable_h
#define test_Integration_Reconfigurable_h
// ======================================================================
//
// ======================================================================

#include "art/Framework/Services/Registry/ActivityRegistry.h"
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

#endif /* test_Integration_Reconfigurable_h */



// Local Variables:
// mode: c++
// End:
