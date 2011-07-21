/*----------------------------------------------------------------------

  ----------------------------------------------------------------------*/

#include "art/Framework/Core/SelectorBase.h"

namespace art
{
  class BranchDescription;

  //------------------------------------------------------------------
  //
  // SelectorBase
  //
  //------------------------------------------------------------------
  SelectorBase::~SelectorBase()
  { }

  bool
  SelectorBase::match(BranchDescription const& p) const
  {
    return doMatch(p);
  }
}
