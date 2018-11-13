#include "art/Utilities/SharedResource.h"
#include "cetlib_except/demangle.h"

namespace art::detail {
  SharedResource_t::SharedResource_t(std::string const& resource_name,
                                     bool const demangle)
    : name{demangle ? cet::demangle_symbol(resource_name) : resource_name}
  {}
}
