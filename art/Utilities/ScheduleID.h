#ifndef art_Utilities_ScheduleID_h
#define art_Utilities_ScheduleID_h

// Entity for identification of schedules and items attached thereto.

#include <cstdint>
#include <limits>
#include <stdexcept>

namespace art {
  class ScheduleID;

  bool operator != (ScheduleID left, ScheduleID right);
}

class art::ScheduleID {
private:
  typedef uint16_t id_type; // Must be unsigned type.
public:
  explicit ScheduleID(id_type id);

  static constexpr id_type min_id();
  static constexpr id_type max_id();

  // Value accessor (use should be rare).
  id_type id() const;

  // Return the next scheduleID;
  ScheduleID next() const { return ScheduleID(id_ + 1); }

  // Comparison operators.
  bool operator == (ScheduleID const & other) const;

private:
  id_type id_;
};

inline
art::ScheduleID::ScheduleID(id_type id)
  : id_((id < min_id() || id > max_id())
        ? throw std::out_of_range("art::ScheduleID: Invalid initializer.")
        : id)
{
}

inline
constexpr
art::ScheduleID::id_type
art::ScheduleID::min_id()
{
  return std::numeric_limits<id_type>::min();
}

inline
constexpr
art::ScheduleID::id_type
art::ScheduleID::max_id()
{
  return std::numeric_limits<id_type>::max() - 1;
}

inline
art::ScheduleID::id_type
art::ScheduleID::id() const
{
  return id_;
}

inline
bool
art::ScheduleID::operator == (ScheduleID const & other) const
{
  return id_ == other.id_;
}

inline
bool
art::operator != (art::ScheduleID left,
                  art::ScheduleID right)
{
  return ! (left == right);
}

#endif /* art_Utilities_ScheduleID_h */

// Local Variables:
// mode: c++
// End:
