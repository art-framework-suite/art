#ifndef art_Framework_Principal_ExecutionCounts_h
#define art_Framework_Principal_ExecutionCounts_h
// vim: set sw=2 expandtab :

#include <cstddef>
#include <tuple>
#include <type_traits>

namespace art {

  namespace stats {

    struct Visited {
      std::size_t value{};
    };

    struct Run {
      std::size_t value{};
    };

    struct Passed {
      std::size_t value{};
    };

    struct Failed {
      std::size_t value{};
    };

    struct ExceptionThrown {
      std::size_t value{};
    };

  } // namespace stats

  template <typename... ARGS>
  class ExecutionCounts {

  public:
    template <typename FIELD>
    std::size_t
    times() const
    {
      return std::get<FIELD>(counts_).value;
    }

    template <typename FIELD>
    void
    increment()
    {
      ++std::get<FIELD>(counts_).value;
    }

    template <typename HEAD_FIELD, typename... TAIL_FIELDS>
    std::enable_if_t<(sizeof...(TAIL_FIELDS) > 0)>
    increment()
    {
      increment<HEAD_FIELD>();
      increment<TAIL_FIELDS...>();
    }

    void
    update(bool const rc)
    {
      if (rc) {
        increment<stats::Passed>();
      } else {
        increment<stats::Failed>();
      }
    }

    void
    reset()
    {
      counts_ = std::tuple<ARGS...>();
    }

  private:
    std::tuple<ARGS...> counts_;
  };

  using CountingStatistics = ExecutionCounts<stats::Visited,
                                             stats::Run,
                                             stats::Passed,
                                             stats::Failed,
                                             stats::ExceptionThrown>;

} // namespace art

#endif /* art_Framework_Principal_ExecutionCounts_h */

// Local variables:
// mode: c++
// End:
