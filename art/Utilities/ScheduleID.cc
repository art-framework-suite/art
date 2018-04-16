// vim: set sw=2 expandtab :
#include "art/Utilities/ScheduleID.h"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <type_traits>

using namespace std;

namespace art {

  ScheduleID::ScheduleID(id_type const id)
  {
    // No need to test against min_id_() since
    // that is the smallest possible value already.
    if (id > max_id_()) {
      // Somebody passed us invalid_id_().
      throw out_of_range("ScheduleID: Invalid initializer.");
    }
    id_ = id;
  }

  ScheduleID
  ScheduleID::next() const
  {
    // Take advantage of the named return value optimization.
    auto ret = ScheduleID(id_ + 1);
    return ret;
  }

  bool
  ScheduleID::operator==(ScheduleID const& rhs) const noexcept
  {
    return id_ == rhs.id_;
  }

  bool
  ScheduleID::operator<(ScheduleID const& rhs) const noexcept
  {
    return id_ < rhs.id_;
  }

  bool
  operator!=(ScheduleID const left, ScheduleID const right) noexcept
  {
    return !(left == right);
  }

  bool
  operator<=(ScheduleID const left, ScheduleID const right) noexcept
  {
    return ((left < right) || (left == right));
  }

  bool
  operator>(ScheduleID const left, ScheduleID const right) noexcept
  {
    return !(left <= right);
  }

  bool
  operator>=(ScheduleID const left, ScheduleID const right) noexcept
  {
    return !(left < right);
  }

  ostream&
  operator<<(ostream& os, ScheduleID const sid)
  {
    return os << sid.id();
  }

} // namespace art
