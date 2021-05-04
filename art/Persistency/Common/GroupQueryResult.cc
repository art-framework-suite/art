// ======================================================================
//
// GroupQueryResult: A pointer-to-const-Group satisfying a query, or an
//                   exception object explaining a failed query.
//
// ======================================================================

#include "art/Persistency/Common/GroupQueryResult.h"

art::GroupQueryResult::GroupQueryResult(group_ptr_t g) : groupOrException_{g} {}

art::GroupQueryResult::GroupQueryResult(exception_ptr_t e)
  : groupOrException_{e}
{}
