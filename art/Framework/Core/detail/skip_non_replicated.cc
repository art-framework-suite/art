#include "art/Framework/Core/detail/skip_non_replicated.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Utilities/ScheduleID.h"

bool
art::detail::skip_non_replicated(Worker const& w)
{
  if (w.scheduleID() == ScheduleID::first()) {
    return false;
  }
  bool const is_replicated{w.description().moduleThreadingType() ==
                           ModuleThreadingType::replicated};
  return !is_replicated;
}
