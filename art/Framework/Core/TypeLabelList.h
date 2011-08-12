#ifndef art_Framework_Core_TypeLabelList_h
#define art_Framework_Core_TypeLabelList_h

#include <set>
#include <string>
#include "art/Utilities/TypeID.h"
#include "art/Persistency/Provenance/BranchType.h"


namespace art
{

  struct TypeLabel
  {
    TypeLabel (BranchType const&  branchType,
               TypeID const&      itemtype,
               std::string const& instanceName,
               std::string const& emulatedMod = std::string()) :
      branchType(branchType),
      typeID(itemtype),
      productInstanceName(instanceName),
      emulatedModule(emulatedMod)
    {}

    std::string className() const { return typeID.className(); }
    std::string friendlyClassName() const { return typeID.friendlyClassName(); }
    bool hasEmulatedModule() const { return !emulatedModule.empty(); }

    BranchType  branchType;
    TypeID      typeID;
    std::string productInstanceName;
    std::string emulatedModule;
  };

  // Types with the same friendlyClassName are in the same equivalence
  // class for the purposes of this comparison.
  inline
  bool operator<(TypeLabel const &a, TypeLabel const &b) {
    return (a.branchType != b.branchType) ? a.branchType < b.branchType :
      (a.emulatedModule != b.emulatedModule) ? a.emulatedModule < b.emulatedModule :
      (a.productInstanceName != b.productInstanceName) ? a.productInstanceName < b.productInstanceName :
      a.friendlyClassName() < b.friendlyClassName();
  }

  typedef std::set<art::TypeLabel> TypeLabelList;

}



#endif /* art_Framework_Core_TypeLabelList_h */

// Local Variables:
// mode: c++
// End:
