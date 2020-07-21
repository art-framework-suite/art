#ifndef art_Framework_Principal_SelectorBase_h
#define art_Framework_Principal_SelectorBase_h

// ==========================================================================
// Selector: Base class for all "selector" objects, used to select
// EDProducts based on information in the associated Provenance.
//
// Developers who make their own Selectors should inherit from SelectorBase.
//
// The 'match' function should return true to denote a successful
// match with the product described by the provided BranchDescription
// object.  Otherwise, the function should return false.  Upon
// returning true, the corresponding product becomes a candidate for
// product retrieval.
//
// The 'print' function is called whenever a product selection fails.
// Selector authors should return a string with the characteristics of
// the selector.  The string will be encapsulated by an error message
// that states the selection criteria were not satisfied during
// product lookup.
//
// For a selector that matches against (e.g.) the product ID, the
// recommended use pattern is:
//
//   return indent + "Product ID: " << product_id;
//
// ==========================================================================

#include "art/Framework/Principal/fwd.h"

#include <string>

namespace art {
  class BranchDescription;
}

class art::SelectorBase {
public:
  virtual ~SelectorBase() = default;
  bool
  match(BranchDescription const& p) const
  {
    return doMatch(p);
  }

  std::string
  print(std::string const& indent) const
  {
    return doPrint(indent);
  }

private:
  virtual bool doMatch(BranchDescription const& p) const = 0;
  virtual std::string doPrint(std::string const& indent) const = 0;
};

#endif /* art_Framework_Principal_SelectorBase_h */

// Local Variables:
// mode: c++
// End:
