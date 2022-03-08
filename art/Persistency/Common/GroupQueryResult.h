#ifndef art_Persistency_Common_GroupQueryResult_h
#define art_Persistency_Common_GroupQueryResult_h

// ======================================================================
//
// GroupQueryResult: A pointer-to-const-Group satisfying a query, or an
//                   exception object explaining a failed query.
//
// ======================================================================

#include "canvas/Utilities/Exception.h"
#include "cetlib/exempt_ptr.h"

#include <memory>
#include <variant>

namespace art {
  class Group;
  class GroupQueryResult;

  using group_ptr_t = cet::exempt_ptr<Group>;
  using exception_ptr_t = std::shared_ptr<art::Exception const>;
} // namespace art

// ======================================================================

class art::GroupQueryResult {
public:
  explicit GroupQueryResult(group_ptr_t);
  explicit GroupQueryResult(exception_ptr_t);

  // observers
  bool
  succeeded() const
  {
    return std::holds_alternative<group_ptr_t>(groupOrException_);
  }
  bool
  failed() const
  {
    return std::holds_alternative<exception_ptr_t>(groupOrException_);
  }

  // properties
  cet::exempt_ptr<Group>
  result() const
  {
    auto result = std::get_if<group_ptr_t>(&groupOrException_);
    return result ? *result : nullptr;
  }
  std::shared_ptr<art::Exception const>
  whyFailed() const
  {
    auto result = std::get_if<exception_ptr_t>(&groupOrException_);
    return result ? *result : nullptr;
  }

private:
  std::variant<group_ptr_t, exception_ptr_t> groupOrException_;
}; // GroupQueryResult

// ======================================================================

#endif /* art_Persistency_Common_GroupQueryResult_h */

// Local Variables:
// mode: c++
// End:
