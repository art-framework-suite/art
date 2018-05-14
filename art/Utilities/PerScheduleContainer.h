#ifndef art_Utilities_PerScheduleContainer_h
#define art_Utilities_PerScheduleContainer_h
// vim: set sw=2 expandtab :

#include "art/Utilities/Globals.h"
#include "art/Utilities/ScheduleID.h"
#include "canvas/Utilities/Exception.h"

#include <utility>
#include <vector>

namespace art {

  template <typename T>
  class PerScheduleContainer {

    static_assert(ScheduleID::first().id() == 0,
                  "First allowed ScheduleID value is not 0.");

  public:
    PerScheduleContainer() = default;
    explicit PerScheduleContainer(ScheduleID::size_type const n) : data_(n) {}

    bool
    is_valid() const
    {
      return !data_.empty();
    }

    auto
    size() const
    {
      return data_.size();
    }

    auto
    cbegin() const noexcept
    {
      return data_.cbegin();
    }

    auto
    begin() const noexcept
    {
      return data_.begin();
    }

    auto
    begin() noexcept
    {
      return data_.begin();
    }

    auto
    cend() const noexcept
    {
      return data_.cend();
    }

    auto
    end() const noexcept
    {
      return data_.end();
    }

    auto
    end() noexcept
    {
      return data_.end();
    }

    // FIXME: Should remove once we replace this with a mappish thing.
    void
    reserve(ScheduleID::size_type const sz)
    {
      data_.reserve(sz);
    }

    // FIXME: Should replace emplace_back with emplace
    template <typename... Args>
    void
    emplace_back(Args&&... args)
    {
      data_.emplace_back(std::forward<Args>(args)...);
    }

    void
    resize(ScheduleID::size_type const sz)
    {
      if (is_valid()) {
        throw Exception{errors::LogicError,
                        "An error occurred while calling "
                        "PerScheduleContainer::expand_to_num_schedules"}
          << "Can only call resize when the container is invalid.\n";
      }
      data_.resize(sz);
    }

    auto
    expand_to_num_schedules()
    {
      if (is_valid()) {
        throw Exception{errors::LogicError,
                        "An error occurred while calling "
                        "PerScheduleContainer::expand_to_num_schedules"}
          << "Can only call expand_to_num_schedules when the "
             "container is invalid.";
      }
      auto const n = Globals::instance()->nschedules();
      data_.resize(n);
      return n;
    }

    T& operator[](ScheduleID const sid) { return data_[sid.id()]; }

    T const& operator[](ScheduleID const sid) const { return data_[sid.id()]; }

    T&
    at(ScheduleID const sid)
    {
      return data_.at(sid.id());
    }

    T const&
    at(ScheduleID const sid) const
    {
      return data_.at(sid.id());
    }

  private:
    std::vector<T> data_;
  };

} // namespace art

#endif /* art_Utilities_PerScheduleContainer_h */

// Local Variables:
// mode: c++
// End:
