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

#include "art/Framework/Core/FCPfwd.h"
#include "art/Framework/Core/TypeLabelList.h"
#include "art/Persistency/Common/Assns.h"
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


namespace art
{
  class EDProduct;
  class ModuleDescription;
  class MasterProductRegistry;
  class ProductRegistryHelper;
}

class art::ProductRegistryHelper
{
public:

  // used by the fwk to register the list of products that are
  // either produced or reconstituted by the object containing this
  // ProductRegistryHelper.
  TypeLabelList& typeLabelList();

  static
  void addToRegistry(TypeLabelList::iterator i,
                     TypeLabelList::iterator e,
                     ModuleDescription const& md,
                     MasterProductRegistry& preg,
                     bool isListener=false);

  // Record the production of an object of type P, with optional
  // instance name, in either the Run or SubRun.
  template <class P, BranchType B>
  TypeLabel const& produces(std::string const& instanceName=std::string());

  // Record the production of an object of type P, with optional
  // instance name, in the Event.
  template <class P>
  TypeLabel const& produces(std::string const& instanceName=std::string());

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

  // Nested utility classes to allow partial specialization
  template <typename PROD, BranchType B = InEvent>
  class Produces;

#ifdef USE_ASSNS_SPEC
  template <typename L, typename R, typename D, BranchType B>
  class Produces<Assns<L, R, D>, B>;

  template <typename L, typename R, BranchType B>
  class Produces<Assns<L, R>, B>;
#endif /* USE_ASSNS_SPEC */

  template <typename PROD, BranchType B>
  friend class Produces;
};

template <typename PROD, art::BranchType B>
class art::ProductRegistryHelper::Produces {
public:
  Produces(ProductRegistryHelper &h) : registrar_(h) {}

  TypeLabel const &operator()(std::string const &instanceName = std::string());

private:
  ProductRegistryHelper &registrar_;
};

#ifdef USE_ASSNS_SPEC
template <typename L, typename R, typename D, art::BranchType B>
class art::ProductRegistryHelper::Produces<art::Assns<L, R, D>, B> {
public:
  Produces(ProductRegistryHelper &h) : registrar_(h) {}

  TypeLabel const &operator()(std::string const &instanceName = std::string());

private:
  ProductRegistryHelper &registrar_;
};

template <typename L, typename R, art::BranchType B>
class art::ProductRegistryHelper::Produces<art::Assns<L, R>, B> {
public:
  Produces(ProductRegistryHelper &h) : registrar_(h) {}

  TypeLabel const &operator()(std::string const &instanceName = std::string());

private:
  ProductRegistryHelper &registrar_;
};
#endif /* USE_ASSNS_SPEC */

// TODO: When we activate C++0x features, we can remove this
// implementation in favor of a default template argument in the
// two-template-parameter member produces.
template <class P>
inline
art::TypeLabel const&
art::ProductRegistryHelper::produces(std::string const& instanceName)
{
  return Produces<P, InEvent>(*this)(instanceName);
}

template <typename P, art::BranchType B>
inline
art::TypeLabel const&
art::ProductRegistryHelper::produces(std::string const& instanceName)
{
  return Produces<P, B>(*this)(instanceName);
}

template <typename P, art::BranchType B>
art::TypeLabel const&
art::ProductRegistryHelper::reconstitutes(std::string const& emulatedModule,
                                     std::string const& instanceName)
{
  verifyInstanceName(instanceName);
  TypeID productType(typeid(P));
  verifyFriendlyClassName(productType.friendlyClassName());
  return reconstitutes_to_branch<B>(productType,
                                    emulatedModule,
                                    instanceName);
}

template <art::BranchType B>
art::TypeLabel const&
art::ProductRegistryHelper::produces_in_branch(TypeID const& productType,
                                          std::string const& instanceName)
{
  TypeLabel tli(B, productType, instanceName);
  typeLabelList_.push_back(tli);
  return typeLabelList_.back();
}

template <art::BranchType B>
art::TypeLabel const&
art::ProductRegistryHelper::reconstitutes_to_branch(TypeID const& productType,
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

template <typename PROD, art::BranchType B>
art::TypeLabel const &
art::ProductRegistryHelper::Produces<PROD, B>::operator()(std::string const &instanceName) {
  verifyInstanceName(instanceName);
  TypeID productType(typeid(PROD));
  verifyFriendlyClassName(productType.friendlyClassName());
  return registrar_.produces_in_branch<B>(productType, instanceName);
}

#ifdef USE_ASSNS_SPEC
// FIXME: this is not a complete implementation of the specialization yet: DO NOT USE!
template <typename L, typename R, typename D, art::BranchType B>
art::TypeLabel const &
art::ProductRegistryHelper::Produces<art::Assns<L, R, D>, B>::operator()(std::string const &instanceName) {
  verifyInstanceName(instanceName);
  TypeID lrType(typeid(art::Assns<L, R, D>));
  TypeID rlType(typeid(art::Assns<L, R, D>));
 // Shouldn't need to verify both versions.
  verifyFriendlyClassName(lrType.friendlyClassName());
  TypeLabel lrTypeLabel = registrar_.produces_in_branch<B>(lrType, instanceName);
  TypeLabel rlTypeLabel = registrar_.produces_in_branch<B>(rlType, instanceName);
  return (lrType.friendlyClassName() < rlType.friendlyClassName()) ?
    lrTypeLabel :
    rlTypeLabel;
}

template <typename L, typename R, art::BranchType B>
art::TypeLabel const &
art::ProductRegistryHelper::Produces<art::Assns<L, R>, B>::operator()(std::string const &instanceName) {
  return Produces<art::Assns<L, R, void>, B>(registrar_)(instanceName);
}
#endif /* USE_ASSNS_SPEC */
#endif /* art_Framework_Core_ProductRegistryHelper_h */

// Local Variables:
// mode: c++
// End:
