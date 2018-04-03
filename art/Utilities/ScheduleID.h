#ifndef art_Utilities_ScheduleID_h
#define art_Utilities_ScheduleID_h
// vim: set sw=2 expandtab :

// Entity for identification of schedules and items attached thereto.

#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace art {
  class ScheduleID;

  bool operator!=(ScheduleID left, ScheduleID right) noexcept;
  bool operator<=(ScheduleID left, ScheduleID right) noexcept;
  bool operator>(ScheduleID left, ScheduleID right) noexcept;
  bool operator>=(ScheduleID left, ScheduleID right) noexcept;
  std::ostream& operator<<(std::ostream&, ScheduleID scheduleID);
} // namespace art

class art::ScheduleID {
private:
  using id_type = uint16_t; // Must be unsigned type.
  static_assert(std::is_unsigned<id_type>::value,
                "ScheduleID::id_type must be unsigned!");

public:
  using size_type = id_type;

  constexpr ScheduleID() noexcept = default; // (invalid).
  explicit ScheduleID(id_type id);           // Non-default c'tor can throw

  // Copy and move construction/assignment will not throw.
  ScheduleID(ScheduleID const&) noexcept = default;
  ScheduleID(ScheduleID&&) noexcept = default;
  ScheduleID& operator=(ScheduleID const&) noexcept = default;
  ScheduleID& operator=(ScheduleID&&) noexcept = default;

  // Validity check.
  constexpr bool isValid() const noexcept;

  // Value accessor (use should be rare).
  constexpr id_type id() const noexcept;

  // Return the next scheduleID.
  ScheduleID next() const;

  // First allowed and last allowed ScheduleIDs.
  static ScheduleID first();
  static ScheduleID last();

  // Comparison operators.
  bool operator==(ScheduleID const& other) const noexcept;
  bool operator<(ScheduleID const& other) const noexcept;

private:
  static constexpr id_type min_id_() noexcept;
  static constexpr id_type max_id_() noexcept;
  static constexpr id_type invalid_id_() noexcept;

  id_type id_{invalid_id_()};
};

inline art::ScheduleID::ScheduleID(id_type const id)
  : id_{(id < min_id_() || id > max_id_()) ?
          throw std::out_of_range("art::ScheduleID: Invalid initializer.") :
          id}
{}

inline constexpr bool
art::ScheduleID::isValid() const noexcept
{
  return !(id_ == invalid_id_());
}

inline constexpr art::ScheduleID::id_type
art::ScheduleID::id() const noexcept
{
  return id_;
}

inline art::ScheduleID
art::ScheduleID::next() const
{
  return ScheduleID(id_ + 1);
}

inline art::ScheduleID
art::ScheduleID::first()
{
  return ScheduleID{min_id_()};
}

inline art::ScheduleID
art::ScheduleID::last()
{
  return ScheduleID{max_id_()};
}

inline bool
art::ScheduleID::operator==(ScheduleID const& other) const noexcept
{
  return id_ == other.id_;
}

inline bool
art::ScheduleID::operator<(ScheduleID const& other) const noexcept
{
  return id_ < other.id_;
}

inline constexpr art::ScheduleID::id_type
art::ScheduleID::min_id_() noexcept
{
  return std::numeric_limits<id_type>::min();
}

inline constexpr art::ScheduleID::id_type
art::ScheduleID::max_id_() noexcept
{
  return invalid_id_() - 1;
}

inline constexpr art::ScheduleID::id_type
art::ScheduleID::invalid_id_() noexcept
{
  return std::numeric_limits<id_type>::max();
}

inline bool
art::operator!=(art::ScheduleID const left,
                art::ScheduleID const right) noexcept
{
  return !(left == right);
}

inline bool
art::operator<=(art::ScheduleID const left,
                art::ScheduleID const right) noexcept
{
  return (left < right || left == right);
}

inline bool
art::operator>(art::ScheduleID const left, art::ScheduleID const right) noexcept
{
  return !(left <= right);
}

inline bool
art::operator>=(art::ScheduleID const left,
                art::ScheduleID const right) noexcept
{
  return !(left < right);
}

inline std::ostream&
art::operator<<(std::ostream& os, art::ScheduleID const sid)
{
  return os << sid.id();
}

#endif /* art_Utilities_ScheduleID_h */

// Local Variables:
// mode: c++
// End:
