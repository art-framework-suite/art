#ifndef art_Utilities_PerThread_h
#define art_Utilities_PerThread_h
// vim: set sw=2 expandtab :

#include "art/Utilities/CurrentProcessingContext.h"

namespace art {

class PerThread {

public: // MEMBER FUNCTIONS -- Special Member Functions

  ~PerThread();

private: // MEMBER FUNCTIONS -- Special Member Functions

  PerThread();

public: // MEMBER FUNCTIONS -- Special Member Functions

  PerThread(PerThread const&) = delete;

  PerThread(PerThread&) = delete;

  PerThread&
  operator=(PerThread const&) = delete;

  PerThread&
  operator=(PerThread&) = delete;

public: // MEMBER FUNCTIONS -- Static API

  static
  PerThread*
  instance();

public: // MEMBER FUNCTIONS -- API for users

  CurrentProcessingContext const&
  getCPC() const;

  void
  setCPC(CurrentProcessingContext const&);

private: // MEMBER DATA

  CurrentProcessingContext
  cpc_{};

};

} // namespace art

// Local Variables:
// mode: c++
// End:

#endif /* art_Utilities_PerThread_h */
