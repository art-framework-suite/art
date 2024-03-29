#include "art/Framework/Principal/Handle.h"
#include <catch2/catch_test_macros.hpp>

#include <concepts>

struct not_a_handle{};

struct valid_handle{
  struct HandleTag{};
};

TEST_CASE("Valid Handle Tests", "[valid_handle_tests]") {
  // standard handle test
  STATIC_REQUIRE(art::detail::is_a_handle<typename art::Handle<int>>);
  // custom handle test
  STATIC_REQUIRE(art::detail::is_a_handle<valid_handle>);
}

TEST_CASE("Invalid Handle Tests", "[invalid_handle_tests]") {
  // invalid built-in type
  STATIC_REQUIRE(!art::detail::is_a_handle<int>);
  // invalid custom type
  STATIC_REQUIRE(!art::detail::is_a_handle<not_a_handle>);
}
