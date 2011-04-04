#ifndef art_Framework_Core_ProductRegistryHelper_h
#define art_Framework_Core_ProductRegistryHelper_h

// -----------------------------------------------------------------
//
// ProductRegistryHelper: This class provides the produces<> templates
// and the reconstitutes<> templates used by other classes to register
// what types of products are put into the Event/SubRun/Run principals
// by the actions of the classes which invoke those templates.
//
// EDProducers' and EDFilters' constructors should call one of the
// 'produces' functions for each product put into the event, subrun,
// or run. Instance names should be provided only when the producer or
// filter in question makes more than one instance of the same type
// per event.
//
// InputSources' constructors should call 'reconstitutes' for each
// product read from an external source if and only if that source
// does not already carry the metadata that art data files carry.
//
// -----------------------------------------------------------------

#include "art/Framework/Core/TypeLabelList.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Utilities/TypeID.h"
#include "art/Utilities/Exception.h"
#include "cetlib/exception.h"

#include <list>
#include <string>

namespace
{
  inline
  void verifyInstanceName(std::string const& instanceName)
  {
    if (instanceName.find('_') != std::string::npos)
      {
	throw art::Exception(art::errors::Configuration)
	  << "Instance name \""
	  << instanceName
	  << "\" is illegal: underscores are not permitted in instance names."
	  << '\n';
      }
  }

  inline
  void verifyFriendlyClassName(std::string const& fcn)
  {
    if (fcn.find('_') != std::string::npos)
      {
	throw art::Exception(art::errors::LogicError)
	  << "Class \""
	  << fcn
	  << "\" is not suitable for use as a product due to the presence of "
	  << "underscores which are not allowed anywhere in the class name "
	  << "(including namespace and enclosing classes).\n";
      }
  }
}


namespace art {
  class EDProduct;
  class ModuleDescription;
  class ProductRegistry;

  class ProductRegistryHelper {
  public:

    // used by the fwk to register the list of products that are
    // either produced or reconstituted by the object containing this
    // ProductRegistryHelper.
    TypeLabelList& typeLabelList();

    static
    void addToRegistry(TypeLabelList::iterator i,
		       TypeLabelList::iterator e,
		       ModuleDescription const& md,
		       ProductRegistry& preg,
		       bool isListener=false);

    // Record the production of an object of type P, with optional
    // instance name, in either the Run or SubRun.
    template <class P, BranchType B>
    TypeLabel const& produces(std::string const& instanceName=std::string());

    // Record the production of an object of type P, with optional
    // instance name, in the Event.
    template <class P>
    TypeLabel const& produces(std::string const& intanceName=std::string());

    // Record the reconstitution of an object of type P, in either the
    // Run, SubRun, or Event, recording that this object was
    // originally created by a module with label modLabel, and with an
    // optional instance name.
    template <class P, BranchType B>
    TypeLabel const& reconstitutes(std::string const& modLabel,
				   std::string const& instanceName=std::string());

  private:

    template <BranchType B>
    TypeLabel const& 
    produces_in_branch(TypeID const& productType,
		       std::string const& instanceName=std::string());

    template <BranchType B>
    TypeLabel const&
    reconstitutes_to_branch(TypeID const& productType,
			    std::string const& moduleLabel,
			    std::string const& instanceName=std::string());

    TypeLabelList typeLabelList_;
  };


  // ---------------------------------------------------------


  // TODO: When we activate C++0x features, we can remove this
  // implementation in favor of a default template argument in the
  // two-template-parameter member produces.
   template <class P>
   inline
   TypeLabel const&
   ProductRegistryHelper::produces(std::string const& instanceName)
   {
     return produces<P, InEvent>(instanceName);
   }

  template <typename P, BranchType B>
  TypeLabel const&
  ProductRegistryHelper::produces(std::string const& instanceName)
  {
    verifyInstanceName(instanceName);
    TypeID productType(typeid(P));
    verifyFriendlyClassName(productType.friendlyClassName());
    return produces_in_branch<B>(productType, instanceName);
  }

  template <typename P, BranchType B>
  TypeLabel const&
  ProductRegistryHelper::reconstitutes(std::string const& emulatedModule,
				       std::string const& instanceName)
  {
    verifyInstanceName(instanceName);
    TypeID productType(typeid(P));
    verifyFriendlyClassName(productType.friendlyClassName());
    return reconstitutes_to_branch<B>(productType,
				      emulatedModule,
				      instanceName);
  }

  template <BranchType B>
  TypeLabel const&
  ProductRegistryHelper::produces_in_branch(TypeID const& productType,
					    std::string const& instanceName)
  {
    TypeLabel tli(B, productType, instanceName);
    typeLabelList_.push_back(tli);
    return typeLabelList_.back();
  }

  template <BranchType B>
  TypeLabel const&
  ProductRegistryHelper::reconstitutes_to_branch(TypeID const& productType,
						 std::string const& emulatedModule,
						 std::string const& instanceName)
  {
    if (emulatedModule.empty())
      throw Exception(errors::Configuration)
	<< "Input sources must call reconstitutes with a non-empty "
	<< "module label.\n";
    TypeLabel tli(B, productType, instanceName, emulatedModule);
    typeLabelList_.push_back(tli);
    return typeLabelList_.back();
  }

}  // art

#endif /* art_Framework_Core_ProductRegistryHelper_h */

// Local Variables:
// mode: c++
// End:
