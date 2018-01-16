#ifndef art_test_Integration_product_aggregation_TaggedValue_h
#define art_test_Integration_product_aggregation_TaggedValue_h

#include "canvas/Utilities/InputTag.h"

namespace arttest {

  template <typename T>
  struct TaggedValue {
    explicit TaggedValue(std::string const& tag, T const value)
      : tag_{tag}, value_{value}
    {}
    art::InputTag const tag_;
    T const value_;
  };
}

#endif
