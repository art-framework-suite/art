#include "catch2/catch_test_macros.hpp"

#include "art/Persistency/Common/PtrMaker.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Common/Ptr.h"

using namespace art;


TEST_CASE("create() concept enforcement"){
  std::string const& instance = "instance";
  get_pid dl;
  PtrMaker<int>::create<dl, std::vector<int>>(dl, instance);
}
