#ifndef art_Utilities_ScheduleIteration_h
#define art_Utilities_ScheduleIteration_h

// ==========================================================================
// ScheduleIteration exists to facilitate simple iteration through the
// configured schedules for an art job, where the user provides a
// callable object that receives a ScheduleID argument.  It is the
// responsibility of the user to provide the correct arguments at
// construction.
//
// Examples:
//
//   ScheduleIteration iteration_a{7u}; // Configured for 7 schedules.
//   ScheduleIteration iteration_b{ScheduleID{2}, ScheduleID{6}};
//
//   auto print_id = [](ScheduleID const sid) { std::cout << sid << ' '; };
//
//   iteration_a.for_each_schedule(print_id); // prints 0, 1, 2, 3, 4, 5, 6
//   iteration_b.for_each_schedule(print_id); // prints 2, 3, 4, 5
// ==========================================================================

#include "art/Utilities/ScheduleID.h"

namespace art {
  class ScheduleIteration {
  public:
    explicit ScheduleIteration(ScheduleID::size_type const n) : end_{n} {}
    explicit ScheduleIteration(ScheduleID const begin, ScheduleID const end)
      : begin_{begin}, end_{end}
    {}
    template <typename F>
    void
    for_each_schedule(F f) const
    {
      for (auto sid = begin_; sid < end_; sid = sid.next()) {
        f(sid);
      }
    }

  private:
    ScheduleID begin_{ScheduleID::first()};
    ScheduleID end_;
  };
}

#endif /* art_Utilities_ScheduleIteration_h */

// Local Variables:
// mode: c++
// End:
