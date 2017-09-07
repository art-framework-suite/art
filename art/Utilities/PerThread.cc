#include "art/Utilities/PerThread.h"
// vim: set sw=2 expandtab :

#include "art/Utilities/CurrentProcessingContext.h"

using namespace std;

namespace art {

PerThread::
~PerThread()
{
}

PerThread::
PerThread()
  : cpc_{}
{
}

PerThread*
PerThread::
instance()
{
  thread_local static PerThread me;
  return &me;
}

CurrentProcessingContext const&
PerThread::
getCPC() const
{
  return cpc_;
}

void
PerThread::
setCPC(CurrentProcessingContext const& cpc)
{
  cpc_ = cpc;
}

} // namespace art

