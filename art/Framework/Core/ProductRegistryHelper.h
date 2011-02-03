#ifndef FWCore_Framework_ProductRegistryHelper_h
#define FWCore_Framework_ProductRegistryHelper_h

/*----------------------------------------------------------------------

ProductRegistryHelper:

----------------------------------------------------------------------*/

#include "art/Framework/Core/TypeLabelList.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Utilities/TypeID.h"
#include "cetlib/exception.h"

#include <list>
#include <string>

namespace art {
  class EDProduct;
  class ModuleDescription;
  class ProductRegistry;

  class ProductRegistryHelper {
  public:

    ProductRegistryHelper() : typeLabelList_() {}
    ~ProductRegistryHelper();


    /// used by the fwk to register the list of products of this module
    TypeLabelList& typeLabelList();

    static
    void addToRegistry(TypeLabelList::iterator i,
		       TypeLabelList::iterator e,
		       ModuleDescription const& md,
		       ProductRegistry& preg,
		       bool isListener=false);

    /// declare what type of product will make and with which optional label
    /** the statement
        \code
	produces<ProductType>("optlabel");
        \endcode
        should be added to the producer ctor for every product */


    template <class ProductType>
    TypeLabel const& produces() {
      return produces<ProductType, InEvent>(std::string());
    }

    template <class ProductType>
    TypeLabel const& produces(std::string const& instanceName) {
      return produces<ProductType, InEvent>(instanceName);
    }

    template <typename ProductType, BranchType B>
    TypeLabel const& produces() {
      return produces<ProductType, B>(std::string());
    }

    template <typename ProductType, BranchType B>
    TypeLabel const& produces(std::string const& instanceName) {
      // ---------------------------------------------------------
      // TODO:
      // test here to make sure that neither the instanceName nor the
      // "friendly class name" contain an underscore. If either does,
      // throw an exception.
      // ---------------------------------------------------------
      TypeID tid(typeid(ProductType));
      if (instanceName.find('_') != std::string::npos) {
	throw cet::exception("BAD_INSTANCE_NAME")
	  << "Instance name \""
	  << instanceName
	  << "\" is illegal: underscores are not permitted in instance names.";
      }
      if (tid.friendlyClassName().find('_') != std::string::npos) {
	throw cet::exception("BAD_CLASS_NAME")
	  << "Class \""
	  << tid.friendlyClassName()
	  << "\" is not suitable for use as a product due to the presence of "
	  << "underscores which are not allowed anywhere in the class name "
	  << "(including namespace and enclosing classes).";
      }
      return produces<B>(tid,instanceName);
    }


    TypeLabel const& produces(const TypeID& id, std::string const& instanceName=std::string()) {
      return produces<InEvent>(id,instanceName);
    }

    template <BranchType B>
    TypeLabel const& produces(const TypeID& id, std::string const& instanceName=std::string()) {
      TypeLabel tli(B, id, instanceName);
      typeLabelList_.push_back(tli);
      return *typeLabelList_.rbegin();
    }
  private:
    TypeLabelList typeLabelList_;
  };

}  // art

#endif

// Local Variables:
// mode: c++
// End:
