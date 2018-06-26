#include "art/Framework/Core/detail/ModuleGraphInfo.h"

std::ostream&
art::detail::operator<<(std::ostream& os, ModuleGraphInfo const& info)
{
  os << "Module type: " << to_string(info.module_type) << '\n';
  os << "Product dependencies: ";
  for (auto const& dep : info.consumed_products) {
    os << dep << ' ';
  }
  os << "\nPaths: ";
  for (auto const& path : info.paths) {
    os << path << ' ';
  }
  return os;
}
