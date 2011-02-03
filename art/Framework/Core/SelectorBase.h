#ifndef art_Framework_Core_SelectorBase_h
#define art_Framework_Core_SelectorBase_h

/*----------------------------------------------------------------------

Selector: Base class for all "selector" objects, used to select
EDProducts based on information in the associated Provenance.

Developers who make their own Selectors should inherit from SelectorBase.



----------------------------------------------------------------------*/

namespace art
{
  class ConstBranchDescription;

  //------------------------------------------------------------------
  //
  //// Abstract base class SelectorBase
  //
  //------------------------------------------------------------------

  class SelectorBase {
  public:
    virtual ~SelectorBase();
    bool match(ConstBranchDescription const& p) const;
    virtual SelectorBase* clone() const = 0;

  private:
    virtual bool doMatch(ConstBranchDescription const& p) const = 0;
  };
}

#endif /* art_Framework_Core_SelectorBase_h */

// Local Variables:
// mode: c++
// End:
