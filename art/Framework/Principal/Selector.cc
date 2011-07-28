/*----------------------------------------------------------------------

  ----------------------------------------------------------------------*/

#include "art/Framework/Principal/Selector.h"

namespace art
{
  //------------------------------------------------------------------
  //
  // Selector
  //
  //------------------------------------------------------------------


  Selector::Selector(Selector const& other) :
    sel_(other.sel_->clone())
  { }

  Selector&
  Selector::operator= (Selector const& other)
  {
    Selector temp(other);
    swap(temp);
    return *this;
  }

  void
  Selector::swap(Selector& other)
  {
    std::swap(sel_, other.sel_);
  }

  // We set sel_ = 0 to help diagnose memory overwrites.
  Selector::~Selector() { delete sel_; sel_ = 0; }

  Selector*
  Selector::clone() const
  {
    return new Selector(*this);
  }


  bool
  Selector::doMatch(BranchDescription const& prov) const
  {
    return sel_->match(prov);
  }

}
