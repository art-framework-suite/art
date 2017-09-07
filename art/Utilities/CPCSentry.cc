#include "art/Utilities/CPCSentry.h"
// vim: set sw=2 expandtab :

#include "art/Utilities/CurrentProcessingContext.h"
#include "art/Utilities/PerThread.h"

namespace art {
namespace detail {

CPCSentry::
~CPCSentry()
{
  PerThread::instance()->setCPC(cpc_);
}

CPCSentry::
CPCSentry(CurrentProcessingContext const& cpc)
  : cpc_(PerThread::instance()->getCPC())
{
  PerThread::instance()->setCPC(cpc);
}

} // namespace detail
} // namespace art

