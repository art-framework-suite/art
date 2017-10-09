// ======================================================================
//
// GroupQueryResult: A pointer-to-const-Group satisfying a query, or an
//                   exception object explaining a failed query.
//
// ======================================================================

#include "art/Persistency/Common/GroupQueryResult.h"

#include <cassert>

using art::GroupQueryResult;

// ----------------------------------------------------------------------
// c'tors:

GroupQueryResult::GroupQueryResult(cet::exempt_ptr<Group const> g) : result_{g}
{
  assert(invariant());
}

GroupQueryResult::GroupQueryResult(std::shared_ptr<art::Exception const> e)
  : whyFailed_{e}
{
  assert(invariant());
}

// ======================================================================
