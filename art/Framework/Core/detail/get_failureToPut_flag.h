#ifndef art_Framework_Core_detail_get_failureToPut_flag_h
#define art_Framework_Core_detail_get_failureToPut_flag_h

/*
  =============================================================

  'get_failureToPut_flag' determines the error handling behavior
  whenever a module that calls 'produces' on a product fails to 'put'
  the product onto the Event.  There are two configuration parameters
  that can be used to govern this behavior:

    Global flag:  "services.scheduler.errorOnFailureToPut" (default 'true')
    Local  flag:  "thisModuleLabel.errorOnFailureToPut"    (default 'true')

  The behavior is as follows:

  (1) If the global flag is set to 'true', the individual module
      instances can override the behavior by setting the local flag to
      'false'.

  (2) If the global flag is 'false', then any attempt to override the
      global flag at the per-module level will be ignored.

  The reason for the asymmetry is motivated by the following use case.
  Consider the configuration:

    services.scheduler.errorOnFailureToPut: true
    physics.producers: {
        p1: @local::experiment.p1
        p2: { ... }
        t1: [p1,p2]
    }

  In this example, "experiment.p1" is owned by the experiment.  Should
  the experiment decide that, for that particular module, it is okay
  to allow a failure to 'put' without throwing an exception, then the
  user's job continues without incident.

  However, if the flags were reversed--i.e. the global flag were set
  to 'false', and the "experiment.p1" flag were set to 'true', the
  user might be surprised to find that their job has failed.  We
  believe this approach is most user-friendly, and it is therefore
  incumbent on particular owners of modules or for software leads of
  experiments to determine reasonable policies for their own
  experiments.

  =============================================================

*/

namespace fhicl {
  class ParameterSet;
}

namespace art::detail {
  bool get_failureToPut_flag(fhicl::ParameterSet const& scheduler_pset,
                             fhicl::ParameterSet const& module_pset);
}

#endif /* art_Framework_Core_detail_get_failureToPut_flag_h */

// Local variables:
// mode: c++
// End:
