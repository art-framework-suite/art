#include "catch2/catch_test_macros.hpp"
#include <array>
#include <concepts>
#include <queue>
#include <stack>
#include <vector>

#include "art/Persistency/Common/CollectionUtilities.h"

using namespace art;

namespace {
  struct two_in_container {
    struct iterator {};
    void
    insert(iterator beg, iterator end)
    {
      (void)beg;
      (void)end;
    }
    iterator
    begin()
    {
      return iterator();
    }
    iterator
    end()
    {
      return iterator();
    }
  };

  struct three_in_container {
    struct iterator {};
    iterator
    begin()
    {
      return iterator();
    }
    iterator
    end()
    {
      return iterator();
    }
    void
    insert(iterator self_end, iterator other_beg, iterator other_end)
    {
      (void)self_end;
      (void)other_beg;
      (void)other_end;
    }
  };

  struct invalid_container {
    void do_nothing(){};
  };

  template <typename T1, typename T2>
  concept can_concat = requires(T1& t1, T2& t2) {
                         requires detail::has_two_arg_insert<T1> ||
                                    detail::has_three_arg_insert<T1>;
                         {
                           concatContainers(t1, t2)
                         };
                       };
}

TEST_CASE("concatContainers() on std containers")
{
  CHECK(can_concat<std::vector<int>, std::vector<int>>);
}

TEST_CASE("concatContainers() on valid custom containers")
{
  CHECK(can_concat<two_in_container,
                   two_in_container>); // Checking with two arg insert
  CHECK(can_concat<three_in_container,
                   three_in_container>); // Checking with three arg insert
}

TEST_CASE("concatContainers() on invalid custom containers")
{
  CHECK_FALSE(can_concat<invalid_container, invalid_container>);
}

TEST_CASE("concatContainers() on std::containers with no `insert()` function")
{
  CHECK_FALSE(can_concat<std::array<int, 1>, std::array<int, 1>>);
}
