#ifndef art_Framework_Core_TypeLabelList_h
#define art_Framework_Core_TypeLabelList_h

#include <list>
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
      emulatedModule(emulatedMod),
      branchAlias()
    {}

    bool hasBranchAlias() const { return !branchAlias.empty(); }
    std::string userClassName() const { return typeID.userClassName(); }
    std::string friendlyClassName() const { return typeID.friendlyClassName(); }
    bool hasEmulatedModule() const { return !emulatedModule.empty(); }

    BranchType  branchType;
    TypeID      typeID;
    std::string productInstanceName;
    std::string emulatedModule;
    std::string branchAlias;
  };

  typedef std::list<art::TypeLabel> TypeLabelList;
}



#endif /* art_Framework_Core_TypeLabelList_h */

// Local Variables:
// mode: c++
// End:
