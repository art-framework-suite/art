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

GroupQueryResult::GroupQueryResult(Group const * g)
  : result_(g)
  , whyFailed_()
{
  assert(invariant());
}

GroupQueryResult::GroupQueryResult(std::shared_ptr<cet::exception const> e)
  : result_(nullptr)
  , whyFailed_(e)
{
  assert(invariant());
}

// ======================================================================
