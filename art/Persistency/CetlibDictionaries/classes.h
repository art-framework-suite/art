#include "cetlib/map_vector.h"
#include "cetlib/sha1.h"

namespace {
  struct dictionary {
    cet::sha1::digest_t d1;
  };
}
