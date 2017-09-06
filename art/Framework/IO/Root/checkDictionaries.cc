#include "art/Framework/IO/Root/checkDictionaries.h"
// vim: set sw=2:

#include "canvas/IO/Root/Common/AssnsStreamer.h"
#include "canvas/IO/Root/Provenance/BranchDescriptionStreamer.h"
#include "canvas/IO/Root/Provenance/DictionaryChecker.h"
#include "canvas/IO/Root/Provenance/TypeTools.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Utilities/WrappedClassName.h"
#include "cetlib/assert_only_one_thread.h"

void
art::checkDictionaries(BranchDescription const& productDesc)
{
  CET_ASSERT_ONLY_ONE_THREAD();

  // Make sure we populate the ROOT-related transient information.  If
  // this is not done, then TBranch-related quantities will not be set
  // correctly.
  detail::BranchDescriptionStreamer::fluffRootTransients(productDesc);
  auto const isTransient = productDesc.transient();

  // Check product dictionaries.
  static root::DictionaryChecker dictChecker{};
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

  // Make sure that the AssnsStreamer is appropropriately setup--must
  // be done after we are sure that there is matching dictionary for
  // the partner.
  if (!is_assns(productDesc.producedClassName())) return;

  detail::AssnsStreamer::init_streamer(productDesc.producedClassName());
}
