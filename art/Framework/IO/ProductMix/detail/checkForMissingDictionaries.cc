#include "art/Framework/IO/ProductMix/detail/checkForMissingDictionaries.h"
#include "canvas_root_io/Utilities/DictionaryChecker.h"

void
art::detail::checkForMissingDictionaries(
  std::vector<TypeID> const& types) noexcept(false)
{
  root::DictionaryChecker checker;
  for (auto const& tid : types) {
    checker.checkDictionaries(tid.className());
  }
  checker.reportMissingDictionaries();
}
