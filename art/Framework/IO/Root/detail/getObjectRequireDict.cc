#include "art/Framework/IO/Root/detail/getObjectRequireDict.h"
#include "canvas_root_io/Utilities/DictionaryChecker.h"

void
art::root::detail::require_dictionary(TypeID const& type) noexcept(false)
{
  DictionaryChecker checker{};
  checker.checkDictionaries(type.className(), true /*recursive check*/);
  checker.reportMissingDictionaries(); // Can throw
}
