/*----------------------------------------------------------------------

  ----------------------------------------------------------------------*/

#include "art/Framework/Core/SelectorBase.h"
#include "art/Persistency/Provenance/ConstBranchDescription.h"

namespace art
{

  //------------------------------------------------------------------
  //
  // SelectorBase
  //
  //------------------------------------------------------------------
  SelectorBase::~SelectorBase()
  { }

  bool
  SelectorBase::match(ConstBranchDescription const& p) const
  {
    return doMatch(p);
  }
}
