#ifndef art_Persistency_Common_GroupQueryResult_h
#define art_Persistency_Common_GroupQueryResult_h

// ======================================================================
//
// GroupQueryResult: A pointer-to-const-Group satisfying a query, or an
//                   exception object explaining a failed query.
//
// ======================================================================

#include "canvas/Utilities/Exception.h"
#include "cetlib/exception.h"
#include "cetlib/exempt_ptr.h"

#include <memory>

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
  GroupQueryResult(cet::exempt_ptr<Group const>);
  GroupQueryResult(std::shared_ptr<art::Exception const>);

  // observers:
  bool succeeded() const { return static_cast<bool>(result_); }
  bool failed() const { return static_cast<bool>(whyFailed_); }

  // properties:
  cet::exempt_ptr<Group const> result() const { return result_; }
  std::shared_ptr<art::Exception const> whyFailed() const { return whyFailed_; }

private:
  cet::exempt_ptr<Group const> result_ {nullptr};
  std::shared_ptr<art::Exception const> whyFailed_ {nullptr};

  bool invariant() const { return succeeded() != failed(); }

};  // GroupQueryResult

// ======================================================================

#endif /* art_Persistency_Common_GroupQueryResult_h */

// Local Variables:
// mode: c++
// End:
