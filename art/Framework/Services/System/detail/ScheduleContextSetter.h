#ifndef TTHTHTHT
#define TTHTHTHT

#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/ScheduleContext.h"

namespace art {
  namespace detail {
    class ScheduleContextSetter;
  }
}

class art::detail::ScheduleContextSetter {
public:
  bool setContext();
  bool resetContext();
private:
  ServiceHandle<ScheduleContext> sh_;
};

inline
bool
art::detail::ScheduleContextSetter::
setContext()
{
  return sh_->setContext();
}

inline
bool
art::detail::ScheduleContextSetter::
resetContext()
{
  return sh_->resetContext();
}

#endif
