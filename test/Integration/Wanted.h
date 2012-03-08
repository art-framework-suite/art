#ifndef LKJH
#define LKJH
// Service with a name near the end of the alphabet with a hook to make
// sure it gets called.

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"

namespace arttest {
  class Wanted;
}

class arttest::Wanted {
public:
  Wanted(fhicl::ParameterSet const &, art::ActivityRegistry &);

  bool postBeginJobCalled() const { return postBeginJobCalled_; }

private:
  void postBeginJob();

  bool postBeginJobCalled_;
};
#endif
