#ifndef art_Utilities_PerScheduleContainer_h
#define art_Utilities_PerScheduleContainer_h
// vim: set sw=2 expandtab :

#include "art/Utilities/Globals.h"
#include "art/Utilities/ScheduleID.h"
#include "canvas/Utilities/Exception.h"
#include "hep_concurrency/RecursiveMutex.h"
#include "hep_concurrency/tsan.h"

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

  public:
    bool
    is_valid() const
    {
      hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
      return !data_.empty();
    }

    auto
    size() const
    {
      hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
      return data_.size();
    }

    auto
    cbegin() const noexcept
    {
      hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
      // Take advantage of the named return value optimization.
      auto ret = data_.cbegin();
      return ret;
    }

    auto
    begin() const noexcept
    {
      hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
      // Take advantage of the named return value optimization.
      auto ret = data_.begin();
      return ret;
    }

    auto
    begin() noexcept
    {
      hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
      // Take advantage of the named return value optimization.
      auto ret = data_.begin();
      return ret;
    }

    auto
    cend() const noexcept
    {
      hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
      // Take advantage of the named return value optimization.
      auto ret = data_.cend();
      return ret;
    }

    auto
    end() const noexcept
    {
      hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
      // Take advantage of the named return value optimization.
      auto ret = data_.end();
      return ret;
    }

    auto
    end() noexcept
    {
      hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
      // Take advantage of the named return value optimization.
      auto ret = data_.end();
      return ret;
    }

    // FIXME: Should replace emplace_back with emplace
    template <typename... Args>
    void
    emplace_back(Args&&... args)
    {
      hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
      data_.emplace_back(std::forward<Args>(args)...);
    }

    void
    resize(ScheduleID::size_type const sz)
    {
      hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
      if (is_valid()) {
        throw Exception{errors::LogicError,
                        "An error occurred while calling "
                        "PerScheduleContainer::expand_to_num_schedules"}
          << "Can only call resize when the "
             "container is invalid.";
      }
      data_.resize(sz);
    }

    auto
    expand_to_num_schedules()
    {
      hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
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

    T& operator[](ScheduleID const sid)
    {
      hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
      // Take advantage of the named return value optimization.
      auto& ret = data_[sid.id()];
      return ret;
    }

    T const& operator[](ScheduleID const sid) const
    {
      hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
      // Take advantage of the named return value optimization.
      auto const& ret = data_[sid.id()];
      return ret;
    }

    T&
    at(ScheduleID const sid)
    {
      hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
      // Take advantage of the named return value optimization.
      auto& ret = data_.at(sid.id());
      return ret;
    }

    T const&
    at(ScheduleID const sid) const
    {
      hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
      // Take advantage of the named return value optimization.
      auto const& ret = data_.at(sid.id());
      return ret;
    }

  private:
    // Protects all data members.
    mutable hep::concurrency::RecursiveMutex mutex_{"art::psc"};

    // The actual container.
    std::vector<T> data_;
  };

} // namespace art

#endif /* art_Utilities_PerScheduleContainer_h */

// Local Variables:
// mode: c++
// End:
