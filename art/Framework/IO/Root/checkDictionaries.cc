#include "art/Framework/IO/Root/checkDictionaries.h"
// vim: set sw=2:

#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/DictionaryChecker.h"
#include "canvas/Persistency/Provenance/TypeTools.h"
#include "canvas/Utilities/WrappedClassName.h"
#include "cetlib/assert_only_one_thread.h"

void
art::checkDictionaries(BranchDescription const& productDesc)
{
  CET_ASSERT_ONLY_ONE_THREAD();
  static DictionaryChecker dictChecker{};

  auto const isTransient = productDesc.transient();

  // Check product dictionaries.
  dictChecker.checkDictionaries(productDesc.wrappedName(), false);
  dictChecker.checkDictionaries(productDesc.producedClassName(), !isTransient);

  // Check dictionaries for assnsPartner, if appropriate. This is only
  // necessary for top-level checks so appropriate here rather than
  // checkDictionaries itself.
  if (!isTransient) {
    auto const assnsPartner = name_of_assns_partner(productDesc.producedClassName());
    if (!assnsPartner.empty()) {
      dictChecker.checkDictionaries(wrappedClassName(assnsPartner), true);
    }
  }
  dictChecker.reportMissingDictionaries();
}
