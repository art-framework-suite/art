#include "catch2/catch_test_macros.hpp"

#include "art/Framework/Principal/Event.h"
#include "art/Persistency/Common/PtrMaker.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Provenance/ProductID.h"

using namespace art;

struct test_struct {
  template <typename CONTAINER>
  ProductID
  getProductID(std::string const& instance)
  {
    (void)instance;
    return ProductID{};
  }

  EDProductGetter const*
  productGetter(ProductID const pid)
  {
    (void)pid;
    return new art::EDProductGetter{};
  }
};

template <typename T, typename CONTAINER>
concept can_create = requires {
                       {
                         can_get_product_id<T, CONTAINER>
                       };
                     };

TEST_CASE("create() concept enforcement")
{
  std::string const& instance = "instance";
  [[maybe_unused]] test_struct const& ts{};

  // Check if 'test_struct' satisfies 'can_get_product_id' concept
  REQUIRE(can_get_product_id<test_struct, std::vector<int>>);

  // Check if 'test_struct' satisfies 'data_level' concept
  REQUIRE(data_level<test_struct, int>);

  // Check if 'test_struct' satisfies 'can_call_productGetter' concept
  REQUIRE(can_call_productGetter<test_struct>);

  PtrMaker<int>::create<std::vector<int>>(ts);
}
