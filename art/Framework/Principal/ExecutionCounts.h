#ifndef art_Framework_Principal_ExecutionCounts_h
#define art_Framework_Principal_ExecutionCounts_h

#include <array>

namespace art {

  // ==============================================
  // Counting-statistics fields

  namespace stats {
    struct Visited { std::size_t value{}; };
    struct Run     { std::size_t value{}; };
    struct Passed  { std::size_t value{}; };
    struct Failed  { std::size_t value{}; };
    struct ExceptionThrown { std::size_t value{}; };
  }

  // ==============================================
  template <typename ... ARGS>
  class ExecutionCounts {
  public:

    template <typename FIELD>
    std::size_t times() const;

    template <bool isEvent>
    void increment() {}

    template <bool isEvent, typename HEAD_FIELD, typename... TAIL_FIELDS>
    void increment();

    template <bool isEvent>
    void update(bool const rc);

    void reset();

  private:
    std::tuple<ARGS...> counts_; // Value-initialized
  };

  // ================================================
  // Type aliases

  using CountingStatistics = ExecutionCounts<stats::Visited,
                                             stats::Run,
                                             stats::Passed,
                                             stats::Failed,
                                             stats::ExceptionThrown>;


  // ==============================================
  // IMPLEMENTATION for ExecutionCounts

  template <typename... ARGS>
  template <typename FIELD>
  std::size_t
  ExecutionCounts<ARGS...>::times() const
  {
    return std::get<FIELD>(counts_).value;
  }

  template <typename... ARGS>
  template <bool isEvent, typename HEAD_FIELD, typename... TAIL_FIELDS>
  void
  ExecutionCounts<ARGS...>::increment()
  {
    if (isEvent) {
      ++std::get<HEAD_FIELD>(counts_).value;
      increment<true, TAIL_FIELDS...>();
    }
  }

  template <typename ... ARGS>
  template <bool isEvent>
  void
  ExecutionCounts<ARGS...>::update(bool const rc)
  {
    if (rc) {
      increment<isEvent,stats::Passed>();
    }
    else {
      increment<isEvent,stats::Failed>();
    }
  }

  template <typename ... ARGS>
  void
  ExecutionCounts<ARGS...>::reset()
  {
    counts_ = std::tuple<ARGS...>();
  }

}

#endif

// Local variables:
// mode: c++
// End:
