#ifndef art_Framework_Core_SelectorBase_h
#define art_Framework_Core_SelectorBase_h

/*----------------------------------------------------------------------

Selector: Base class for all "selector" objects, used to select
EDProducts based on information in the associated Provenance.

Developers who make their own Selectors should inherit from SelectorBase.



----------------------------------------------------------------------*/

namespace art
{
  class BranchDescription;

  //------------------------------------------------------------------
  //
  //// Abstract base class SelectorBase
  //
  //------------------------------------------------------------------

  class SelectorBase {
  public:
    virtual ~SelectorBase();
    bool match(BranchDescription const& p) const;
    virtual SelectorBase* clone() const = 0;

  private:
    virtual bool doMatch(BranchDescription const& p) const = 0;
  };
}

#endif /* art_Framework_Core_SelectorBase_h */

// Local Variables:
// mode: c++
// End:
