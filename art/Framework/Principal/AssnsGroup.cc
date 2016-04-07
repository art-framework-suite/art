#include "art/Framework/Principal/AssnsGroup.h"
// vim: set sw=2:

#include "cetlib/demangle.h"

#include <iostream>

art::AssnsGroup::
AssnsGroup()
  : Group()
  , secondary_wrapper_type_()
  , secondaryProduct_()
{
}

art::AssnsGroup::
AssnsGroup(BranchDescription const& bd,
           ProductID const& pid,
           TypeID const& primary_wrapper_type,
           TypeID const& secondary_wrapper_type,
           ProductRangeSetLookup& prsl,
           cet::exempt_ptr<Worker> productProducer,
           cet::exempt_ptr<EventPrincipal> onDemandPrincipal)
  : Group{bd, pid, primary_wrapper_type, prsl, productProducer, onDemandPrincipal}
  , secondary_wrapper_type_(secondary_wrapper_type)
  , secondaryProduct_()
{}

art::AssnsGroup::
AssnsGroup(std::unique_ptr<EDProduct>&& edp,
           BranchDescription const& bd,
           ProductID const& pid,
           TypeID const& primary_wrapper_type,
           TypeID const& secondary_wrapper_type,
           ProductRangeSetLookup& prsl)
  : Group{std::move(edp), bd, pid, primary_wrapper_type, prsl}
  , secondary_wrapper_type_(secondary_wrapper_type)
  , secondaryProduct_()
{
}

art::EDProduct const*
art::AssnsGroup::
getIt() const
{
  return uniqueProduct();
}

art::EDProduct const*
art::AssnsGroup::
anyProduct() const
{
  return secondaryProduct_ ?  secondaryProduct_.get() : Group::uniqueProduct();
}

art::EDProduct const*
art::AssnsGroup::
uniqueProduct() const
{
  throw Exception(errors::LogicError, "AmbiguousProduct")
      << "AssnsGroup was asked for a held product (uniqueProduct()) "
      << "without specifying which one was wanted.\n";
}

art::EDProduct const*
art::AssnsGroup::
uniqueProduct(TypeID const& wanted_wrapper_type) const
{
  //std::cout
  //    << "-----> Begin AssnsGroup::uniqueProduct(TypeID const&):"
  //    << endl
  //    << "wt: "
  //    << cet::demangle_symbol(wanted_wrapper_type.name())
  //    << endl;
  EDProduct const* retval = nullptr;
  if (wanted_wrapper_type == secondary_wrapper_type_) {
    //std::cout
    //    << "in wanted_wrapper_type == secondary_wrapper_type_ case"
    //    << endl
    //    << "using secondaryProduct_.get()"
    //    << endl;
    retval = secondaryProduct_.get();
  }
  else {
    //std::cout
    //    << "in wanted_wrapper_type != secondary_wrapper_type_ case"
    //    << endl
    //    << "calling up to the Group::uniqueProduct()"
    //    << endl
    //    << "which means using product_.get()"
    //    << endl;
    retval = Group::uniqueProduct();
  }
  //std::cout
  //    << "returning: "
  //    << retval
  //    << endl;
  //std::cout
  //    << "-----> End   AssnsGroup::uniqueProduct(TypeID const&):"
  //    << endl;
  return retval;
}

bool
art::AssnsGroup::
resolveProductIfAvailable(bool fillOnDemand, TypeID const& wanted_wrapper_type) const
{
  //std::cout
  //    << "-----> Begin AssnsGroup::resolveProductIfAvailable(...)"
  //    << endl
  //    << "wanted_wrapper_type:     "
  //    << cet::demangle_symbol(wanted_wrapper_type.name())
  //    << endl
  //    << "secondary_wrapper_type_: "
  //    << cet::demangle_symbol(secondary_wrapper_type_.name())
  //    << endl;
  if (uniqueProduct(wanted_wrapper_type) != nullptr) {
    // Nothing to do.
    //std::cout
    //    << "already have it"
    //    << endl
    //    << "returning: true"
    //    << endl
    //    << "-----> End   AssnsGroup::resolveProductIfAvailable(...)"
    //    << endl;
    return true;
  }
  if (productUnavailable()) {
    // Nothing we *can* do.
    //std::cout
    //    << "product is not available"
    //    << endl
    //    << "returning: false"
    //    << endl
    //    << "-----> End   AssnsGroup::resolveProductIfAvailable(...)"
    //    << endl;
    return false;
  }
  // We know at this point that our wanted object has not
  // been read or created yet.
  std::unique_ptr<EDProduct> edp;
  if (wanted_wrapper_type == secondary_wrapper_type_) {
    //std::cout
    //    << "we want the secondary wrapper type"
    //    << endl;
    if (Group::uniqueProduct() == nullptr) {
      // Our partner needs to be read or
      // demand produced first.
      //std::cout
      //    << "produced object not read yet, trying to obtain it"
      //    << endl;
      edp = obtainDesiredProduct(fillOnDemand, producedWrapperType());
      if (edp.get()) {
        //std::cout
        //    << "produced object read"
        //    << endl
        //    << "calling setProduct to set product_ to: "
        //    << edp.get()
        //    << endl;
        setProduct(std::move(edp));
      }
      else {
        //std::cout
        //    << "failed to get the produced object"
        //    << endl;
      }
    }
    if (Group::uniqueProduct() != nullptr) {
      // Our partner has already been read, so call its
      // makePartner function to get what we want.
      //std::cout
      //    << "we have the produced product, now calling"
      //    << endl
      //    << "its makeParner to get what we want"
      //    << endl;
      edp = Group::uniqueProduct()->makePartner(wanted_wrapper_type.typeInfo());
      if (edp.get() != nullptr) {
        //std::cout
        //    << "setting secondaryProduct_ to: "
        //    << edp.get()
        //    << endl;
        secondaryProduct_ = std::move(edp);
      }
      else {
        //std::cout
        //    << "makePartner failed to make what we want"
        //    << endl;
      }
    }
  }
  else {
    // We want the produced type.
    //std::cout
    //    << "we want the produced wrapper type"
    //    << endl;
    //std::cout
    //    << "produced object not read yet, trying to obtain it"
    //    << endl;
    edp = obtainDesiredProduct(fillOnDemand, producedWrapperType());
    if (edp.get()) {
      //std::cout
      //    << "produced wrapper type product found"
      //    << endl
      //    << "calling setProduct to set product_ to: "
      //    << edp.get()
      //    << endl;
      setProduct(std::move(edp));
    }
    else {
      //std::cout
      //    << "failed to get the produced object"
      //    << endl;
    }
  }
  bool retval = false;
  if (uniqueProduct(wanted_wrapper_type) != nullptr) {
    retval = true;
  }
  //std::cout
  //    << "returning: "
  //    << retval
  //    << endl;
  //std::cout
  //    << "-----> End   AssnsGroup::resolveProductIfAvailable(...)"
  //    << endl;
  return retval;
}
