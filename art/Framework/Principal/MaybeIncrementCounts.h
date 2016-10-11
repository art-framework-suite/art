#ifndef art_Framework_Principal_MaybeIncrementCounts_h
#define art_Framework_Principal_MaybeIncrementCounts_h

#include "art/Framework/Principal/ExecutionCounts.h"
#include "canvas/Persistency/Provenance/IDNumber.h"

namespace art {
  template <Level, typename T>
  class MaybeIncrementCounts {
  public:
    MaybeIncrementCounts(T&){}

    template <typename... ARGS>
    void increment() {}
    void update(bool const) {}
  };

  template <typename T>
  class MaybeIncrementCounts<Level::Event, T> {
  public:
    MaybeIncrementCounts(T& t) : t_{t} {}

    template <typename... ARGS>
    void increment() { t_.template increment<ARGS...>(); }
    void update(bool const rc) { t_.update(rc); }
  private:
    T& t_;
  };
}

#endif

// Local variables:
// mode: c++
// End:
