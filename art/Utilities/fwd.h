#ifndef art_Utilities_fwd_h
#define art_Utilities_fwd_h
////////////////////////////////////////////////////////////////////////
// Fowrard declarations for art/Utilities.
////////////////////////////////////////////////////////////////////////

// These headers are required to forward-declare adequately types and
// contain *only* what is required so to do.
#include "art/Utilities/Verbosity.h"
#include "canvas/Utilities/Exception.h"

namespace art {
  class InputTag;
  struct MallocOpts;
  class MallocOptionsSetter; // MallocOpts.h
  class RootHandlers;
  class TypeID;
} // namespace art
#endif /* art_Utilities_fwd_h */

// Local Variables:
// mode: c++
// End:
