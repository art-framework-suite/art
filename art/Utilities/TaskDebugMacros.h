#ifndef art_Utilities_TaskDebugMacros_h
#define art_Utilities_TaskDebugMacros_h
// vim: set sw=2 expandtab :

#include "art/Utilities/ScheduleID.h"
#include "hep_concurrency/tsan.h"

#include <atomic>
#include <iostream>
#include <sstream>
#include <string>

namespace art {

  struct DebugTasksValue {
    DebugTasksValue();

    int
    operator()() const noexcept
    {
      return value_.load();
    }

    std::atomic<char const*> cvalue_;
    std::atomic<int> value_;
  };

  extern DebugTasksValue debugTasks;

  namespace detail {
    class MessageAccumulator {
    public:
      MessageAccumulator(char const banner_prefix,
                         std::string const& fcn_name,
                         std::string const& pretty_fcn_name,
                         ScheduleID schedule_id = ScheduleID{},
                         std::string const& step = std::string(6, ' '));
      ~MessageAccumulator();

      template <typename T>
      decltype(auto)
      operator<<(T const& t)
      {
        usr_msg_ << t;
        return std::forward<decltype(*this)>(*this);
      }

    private:
      std::ostringstream buffer_;
      std::ostringstream usr_msg_;
    };
  }
}

#define TDEBUG(LEVEL)                                                          \
  if ((LEVEL) <= art::debugTasks())                                            \
  std::cerr

#define TDEBUG_BEGIN_FUNC_SI(LEVEL, SI)                                        \
  if ((LEVEL) <= art::debugTasks())                                            \
  art::detail::MessageAccumulator(                                             \
    '-', __func__, __PRETTY_FUNCTION__, SI, "Begin")

#define TDEBUG_END_FUNC_SI(LEVEL, SI)                                          \
  if ((LEVEL) <= art::debugTasks())                                            \
  art::detail::MessageAccumulator('-', __func__, __PRETTY_FUNCTION__, SI, "End")

#define TDEBUG_FUNC_SI(LEVEL, SI)                                              \
  if ((LEVEL) <= art::debugTasks())                                            \
  art::detail::MessageAccumulator('-', __func__, __PRETTY_FUNCTION__, SI)

#define TDEBUG_FUNC(LEVEL)                                                     \
  if ((LEVEL) <= art::debugTasks())                                            \
  art::detail::MessageAccumulator('-', __func__, __PRETTY_FUNCTION__)

#define TDEBUG_TASK_SI(LEVEL, SI)                                              \
  if ((LEVEL) <= art::debugTasks())                                            \
  art::detail::MessageAccumulator('=', __func__, __PRETTY_FUNCTION__, SI)

#define TDEBUG_TASK(LEVEL)                                                     \
  if ((LEVEL) <= art::debugTasks())                                            \
  art::detail::MessageAccumulator('=', __func__, __PRETTY_FUNCTION__)

#define TDEBUG_BEGIN_TASK_SI(LEVEL, SI)                                        \
  if ((LEVEL) <= art::debugTasks())                                            \
  art::detail::MessageAccumulator(                                             \
    '=', __func__, __PRETTY_FUNCTION__, SI, "Begin")

#define TDEBUG_END_TASK_SI(LEVEL, SI)                                          \
  if ((LEVEL) <= art::debugTasks())                                            \
  art::detail::MessageAccumulator('=', __func__, __PRETTY_FUNCTION__, SI, "End")

#endif /* art_Utilities_TaskDebugMacros_h */

// Local Variables:
// mode: c++
// End:
