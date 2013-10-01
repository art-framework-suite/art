#ifndef art_Utilities_SplitService_h
#define art_Utilities_SplitService_h
////////////////////////////////////////////////////////////////////////
// SplitService
//
// Empty "flag" class to indicate a copy-like constructor for *local*
// services. If the service provides a copy-like constructor:
//
// MyService(MyService const & other, art::Split);
//
// Then it will be used in preference to the normal, macro-required
// constructor to create the second and subsequent services from the
// first.
//
// Modeled after tbb::split, used for parallel_reduce().
//
////////////////////////////////////////////////////////////////////////

namespace art {
  class SplitService;
}

class art::SplitService {
};

#endif /* art_Utilities_SplitService_h */

// Local Variables:
// mode: c++
// End:
