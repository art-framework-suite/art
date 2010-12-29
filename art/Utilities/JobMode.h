#ifndef FWCore_Utilities_JobMode_h
#define FWCore_Utilities_JobMode_h

/*
 An enum indicating the nature of the job, for use (at least initially)
 in deciding what the "hardwired" defaults for MessageLogger configuration
 ought to be.

*/

namespace art {

  enum JobMode {
         GridJobMode
       , ReleaseValidationJobMode
       , AnalysisJobMode
       , NilJobMode
  };

}  // art

// ======================================================================

#endif
