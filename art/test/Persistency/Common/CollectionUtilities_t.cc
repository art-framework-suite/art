#include "catch2/catch_test_macros.hpp"
#include <vector>
#include <concepts>

#include "art/Persistency/Common/CollectionUtilities.h"

using namespace art;

namespace {
  template <typename T>
  struct two_in_container{
    void insert(typename T::iterator beg, typename T::iterator end){}
  };

  template <typename T>
  struct three_in_container{
    struct iterator {};
    iterator begin() {
      return iterator();
    }
    void insert(typename T::iterator self_end, typename T::iterator other_beg, typename T::iterator other_end){}
  };

  template <typename T1, typename T2>
  concept can_concat = requires (T1& t1, T2& t2) {
    { concatContainers(t1, t2) };
  };
}

TEST_CASE("ensuring proper constraint enforcement in conicatContainers()"){
  CHECK(can_concat<std::vector<int>, std::vector<int>>);
}
