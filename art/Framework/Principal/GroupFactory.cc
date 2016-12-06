#include "art/Framework/Principal/GroupFactory.h"
// vim: set sw=2:

#include "canvas/Persistency/Common/Assns.h"
#include "canvas/Persistency/Provenance/TypeTools.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/WrappedClassName.h"
#include "cetlib/demangle.h"
#include "TClass.h"

using namespace std;

namespace {
  void maybeThrowLateDictionaryError(art::TypeWithDict const & twd,
                                     std::string const & tname)
  {
    if (!twd) {
      art::throwLateDictionaryError(tname);
    }
  }
}

std::vector<art::TypeID>
art::gfactory::detail::
getWrapperTIDs(BranchDescription const& bd)
{
  std::vector<TypeID> result;
  auto const taName = bd.producedClassName();
  TypeWithDict const ta(taName);
  maybeThrowLateDictionaryError(ta, taName);
  auto const twName = art::wrappedClassName(taName);
  TypeWithDict const tw(twName);
  maybeThrowLateDictionaryError(tw, twName);
  result.emplace_back(tw.id()); // Wrapper.
  if (ta.category() == TypeWithDict::Category::CLASSTYPE) {
    if (is_instantiation_of(ta.tClass(), "art::Assns")) {
      auto const tpName = name_of_assns_partner(taName);
      TypeWithDict const tp(tpName);
      maybeThrowLateDictionaryError(tp, tpName);
      auto const twpName = art::wrappedClassName(tpName);
      TypeWithDict const twp(twpName);
      maybeThrowLateDictionaryError(twp, twpName);
      result.emplace_back(twp.id()); // Wrapper of partner.
      auto const baseName = name_of_assns_base(taName);
      if (!baseName.empty()) {
        TypeWithDict const base(baseName);
        maybeThrowLateDictionaryError(base, baseName);
        auto const basewName = art::wrappedClassName(baseName);
        TypeWithDict const basew(basewName);
        maybeThrowLateDictionaryError(basew, basewName);
        result.emplace_back(basew.id()); // Wrapper of base.
        auto const basepName = name_of_assns_partner(baseName);
        TypeWithDict const basep(basepName);
        maybeThrowLateDictionaryError(basep, basepName);
        auto const basewpName = art::wrappedClassName(basepName);
        TypeWithDict const basewp(basewpName);
        maybeThrowLateDictionaryError(basewp, basewpName);
        result.emplace_back(basewp.id()); // Wrapper of partner's base.
      }
    }
  }
  return result;
}
