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

  // A severely limited version of uint16_t intended as the index
  // into a PerScheduleContainer.
  class ScheduleID {
    template <typename T>
    friend class PerScheduleContainer;

  public: // Types
    using id_type = uint16_t;
    // Must be unsigned type, enforce this.
    static_assert(std::is_unsigned<id_type>::value,
                  "ScheduleID::id_type must be unsigned!");
    using size_type = id_type;

  private: // Types
    enum class PremadeTypeFlag { First, Last };

  private: // Static API -- Implementation Details -- Constants
    static constexpr id_type
    invalid_id_() noexcept
    {
      return std::numeric_limits<id_type>::max();
    }
    static constexpr id_type
    min_id_() noexcept
    {
      return std::numeric_limits<id_type>::min();
    }
    static constexpr id_type
    max_id_() noexcept
    {
      return invalid_id_() - 1;
    }

  public: // Static API for the user -- Constants
    // First allowed and last allowed ScheduleIDs.
    static constexpr ScheduleID
    first()
    {
      return ScheduleID{PremadeTypeFlag::First};
    }
    static constexpr ScheduleID
    last()
    {
      return ScheduleID{PremadeTypeFlag::Last};
    }

  public: // Special Member Functions
    constexpr ScheduleID() noexcept = default;
    explicit ScheduleID(id_type id);
    ScheduleID(ScheduleID const&) noexcept = default;
    ScheduleID(ScheduleID&&) noexcept = default;
    ScheduleID& operator=(ScheduleID const&) noexcept = default;
    ScheduleID& operator=(ScheduleID&&) noexcept = default;

  public: // API for the user.
    // Validity check.
    constexpr bool
    isValid() const noexcept
    {
      return !(id_ == invalid_id_());
    }
    // Value accessor (use should be rare).
    constexpr id_type
    id() const noexcept
    {
      return id_;
    }
    // Return the next scheduleID.
    ScheduleID next() const;
    // Comparison operators.
    bool operator==(ScheduleID const& other) const noexcept;
    bool operator<(ScheduleID const& other) const noexcept;

  private: // Implementation Details
    constexpr ScheduleID(PremadeTypeFlag flag)
      : id_{(flag == PremadeTypeFlag::First) ? min_id_() : max_id_()}
    {}

  private: // Data Members
    id_type id_{invalid_id_()};
  };

  bool operator!=(ScheduleID left, ScheduleID right) noexcept;
  bool operator<=(ScheduleID left, ScheduleID right) noexcept;
  bool operator>(ScheduleID left, ScheduleID right) noexcept;
  bool operator>=(ScheduleID left, ScheduleID right) noexcept;
  std::ostream& operator<<(std::ostream&, ScheduleID scheduleID);

  std::size_t tbb_hasher(ScheduleID);

} // namespace art

#endif /* art_Utilities_ScheduleID_h */

// Local Variables:
// mode: c++
// End:
