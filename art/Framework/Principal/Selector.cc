#include "art/Framework/Principal/Selector.h"
// vim: set sw=2 expandtab :

namespace art {

  bool
  Selector::doMatch(BranchDescription const& prov) const
  {
    return sel_->match(prov);
  }

  std::string
  Selector::doPrint(std::string const& indent) const
  {
    return sel_->print(indent);
  }

} // namespace art
