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

    template <typename HEAD_FIELD>
    void increment();

    template <typename HEAD_FIELD, typename... TAIL_FIELDS>
    std::enable_if_t<(sizeof...(TAIL_FIELDS)>0)> increment();

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
  template <typename FIELD>
  void
  ExecutionCounts<ARGS...>::increment()
  {
    ++std::get<FIELD>(counts_).value;
  }

  template <typename... ARGS>
  template <typename HEAD_FIELD, typename... TAIL_FIELDS>
  std::enable_if_t<(sizeof...(TAIL_FIELDS)>0)>
  ExecutionCounts<ARGS...>::increment()
  {
    increment<HEAD_FIELD>();
    increment<TAIL_FIELDS...>();
  }


  template <typename ... ARGS>
  void
  ExecutionCounts<ARGS...>::update(bool const rc)
  {
    if (rc) {
      increment<stats::Passed>();
    }
    else {
      increment<stats::Failed>();
    }
  }

  template <typename ... ARGS>
  void
  ExecutionCounts<ARGS...>::reset()
  {
    counts_ = std::tuple<ARGS...>();
  }

}

#endif /* art_Framework_Principal_ExecutionCounts_h */

// Local variables:
// mode: c++
// End:
