/*----------------------------------------------------------------------

  ----------------------------------------------------------------------*/

#include "art/Framework/Core/Selector.h"

namespace edm
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
  Selector::doMatch(ConstBranchDescription const& prov) const
  {
    return sel_->match(prov);
  }

}
