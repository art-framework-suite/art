#include "art/Framework/Core/detail/ScheduleTask.h"
#include "art/Framework/Services/System/ScheduleContext.h"
#include "art/Utilities/ScheduleID.h"

#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"
#include "tbb/task.h"
#include "tbb/task_scheduler_init.h"

#include <cassert>
#include <memory>

namespace {
  art::ScheduleContext scs;

  void
  yesfunc()
  {
    assert(scs.currentScheduleID().isValid());
  }
  void
  nofunc()
  {
    assert(!scs.currentScheduleID().isValid());
  }
  class NestedTask : public tbb::task {
  public:
    tbb::task*
    execute() override
    {
      assert(scs.currentScheduleID().isValid());
      yesfunc();
      typedef tbb::blocked_range<size_t> range_t;
      tbb::parallel_for(range_t(0, 3), [](range_t const&) { nofunc(); });
      return nullptr;
    }
  };
}

int
main()
{
  tbb::task_scheduler_init tbb;
  std::shared_ptr<tbb::task> topTask{
    new (tbb::task::allocate_root())
      art::detail::ScheduleTask{art::ScheduleID::first()},
    [](tbb::task* iTask) { tbb::task::destroy(*iTask); }};
  assert(!scs.currentScheduleID().isValid()); // Should safely return invalid.
  topTask->set_ref_count(2);
  scs.setContext();
  tbb::task::spawn(*(new (topTask->allocate_child()) NestedTask()));
  topTask->wait_for_all();
  scs.resetContext();
}
