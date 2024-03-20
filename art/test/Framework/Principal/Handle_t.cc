#include "art/Framework/Principal/Handle.h"

#include <concepts>

struct not_a_handle{};

struct valid_handle{
  struct HandleTag{};
};

