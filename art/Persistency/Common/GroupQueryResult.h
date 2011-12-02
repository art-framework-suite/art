#ifndef art_Persistency_Common_GroupQueryResult_h
#define art_Persistency_Common_GroupQueryResult_h

// ======================================================================
//
// GroupQueryResult: A pointer-to-const-Group satisfying a query, or an
//                   exception object explaining a failed query.
//
// ======================================================================

#include "cetlib/exception.h"
#include "cetlib/exempt_ptr.h"
#include "cpp0x/memory"
#include <cassert>

namespace art {
  // defined below:
  class GroupQueryResult;

  // forward declaration:
  class Group;
}

// ======================================================================

class art::GroupQueryResult
{
public:
  // c'tors:
  GroupQueryResult( Group const * );
  GroupQueryResult( std::shared_ptr<cet::exception const> );

  // use compiler-generated copy c'tor, copy assignment, and d'tor

  // observers:
  bool  succeeded( ) const  { return static_cast<bool>(result_); }
  bool  failed   ( ) const  { return static_cast<bool>(whyFailed_); }

  // properties:
  cet::exempt_ptr<Group const>           result   ( ) const  { return result_; }
  std::shared_ptr<cet::exception const>  whyFailed( ) const  { return whyFailed_; }

private:
  cet::exempt_ptr<Group const>           result_;
  std::shared_ptr<cet::exception const>  whyFailed_;

  bool  invariant( )  const  { return succeeded() != failed(); }

};  // GroupQueryResult

// ======================================================================

#endif /* art_Persistency_Common_GroupQueryResult_h */

// Local Variables:
// mode: c++
// End:
