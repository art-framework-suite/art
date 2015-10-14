#include "art/Framework/Principal/AssnsGroup.h"
// vim: set sw=2:

#include "cetlib/demangle.h"

#include <iostream>

using namespace std;

namespace art {

AssnsGroup::
AssnsGroup()
  : Group()
  , secondary_wrapper_type_()
  , secondaryProduct_()
{
}

AssnsGroup::
AssnsGroup(BranchDescription const& bd, ProductID const& pid, TypeID const& primary_wrapper_type, TypeID const& secondary_wrapper_type,
           cet::exempt_ptr<Worker> productProducer, cet::exempt_ptr<EventPrincipal> onDemandPrincipal)
  : Group(bd, pid, primary_wrapper_type, productProducer, onDemandPrincipal)
  , secondary_wrapper_type_(secondary_wrapper_type)
  , secondaryProduct_()
{
  //cout
  //    << "-----> Begin AssnsGroup::AssnsGroup(bd,pid,pwt,swt,wrk,ep)"
  //    << endl
  //    << "pwt:  "
  //    << cet::demangle_symbol(primary_wrapper_type.name())
  //    << endl
  //    << "swt:  "
  //    << cet::demangle_symbol(secondary_wrapper_type.name())
  //    << endl
  //    << "swt_: "
  //    << cet::demangle_symbol(secondary_wrapper_type_.name())
  //    << endl;
  //cout
  //    << "-----> End   AssnsGroup::AssnsGroup(bd,pid,pwt,swt,wrk,ep)"
  //    << endl;
}

AssnsGroup::
AssnsGroup(unique_ptr<EDProduct>&& edp, BranchDescription const& bd, ProductID const& pid, TypeID const& primary_wrapper_type,
           TypeID const& secondary_wrapper_type)
  : Group(move(edp), bd, pid, primary_wrapper_type)
  , secondary_wrapper_type_(secondary_wrapper_type)
  , secondaryProduct_()
{
}

EDProduct const*
AssnsGroup::
getIt() const
{
  return uniqueProduct();
}

EDProduct const*
AssnsGroup::
anyProduct() const
{
  return secondaryProduct_ ?  secondaryProduct_.get() : Group::uniqueProduct();
}

EDProduct const*
AssnsGroup::
uniqueProduct() const
{
  throw Exception(errors::LogicError, "AmbiguousProduct")
      << "AssnsGroup was asked for a held product (uniqueProduct()) "
      << "without specifying which one was wanted.\n";
}

EDProduct const*
AssnsGroup::
uniqueProduct(TypeID const& wanted_wrapper_type) const
{
  //cout
  //    << "-----> Begin AssnsGroup::uniqueProduct(TypeID const&):"
  //    << endl
  //    << "wt: "
  //    << cet::demangle_symbol(wanted_wrapper_type.name())
  //    << endl;
  EDProduct const* retval = nullptr;
  if (wanted_wrapper_type == secondary_wrapper_type_) {
    //cout
    //    << "in wanted_wrapper_type == secondary_wrapper_type_ case"
    //    << endl
    //    << "using secondaryProduct_.get()"
    //    << endl;
    retval = secondaryProduct_.get();
  }
  else {
    //cout
    //    << "in wanted_wrapper_type != secondary_wrapper_type_ case"
    //    << endl
    //    << "calling up to the Group::uniqueProduct()"
    //    << endl
    //    << "which means using product_.get()"
    //    << endl;
    retval = Group::uniqueProduct();
  }
  //cout
  //    << "returning: "
  //    << retval
  //    << endl;
  //cout
  //    << "-----> End   AssnsGroup::uniqueProduct(TypeID const&):"
  //    << endl;
  return retval;
}

bool
AssnsGroup::
resolveProductIfAvailable(bool fillOnDemand, TypeID const& wanted_wrapper_type) const
{
  /**/cout
  /**/    << "-----> Begin AssnsGroup::resolveProductIfAvailable(...)"
  /**/    << endl
  /**/    << "wanted_wrapper_type:     "
  /**/    << cet::demangle_symbol(wanted_wrapper_type.name())
  /**/    << endl
  /**/    << "secondary_wrapper_type_: "
  /**/    << cet::demangle_symbol(secondary_wrapper_type_.name())
  /**/    << endl;
  if (uniqueProduct(wanted_wrapper_type) != nullptr) {
    // Nothing to do.
    /**/cout
    /**/    << "already have it"
    /**/    << endl
    /**/    << "returning: true"
    /**/    << endl
    /**/    << "-----> End   AssnsGroup::resolveProductIfAvailable(...)"
    /**/    << endl;
    return true;
  }
  if (productUnavailable()) {
    // Nothing we *can* do.
    /**/cout
    /**/    << "product is not available"
    /**/    << endl
    /**/    << "returning: false"
    /**/    << endl
    /**/    << "-----> End   AssnsGroup::resolveProductIfAvailable(...)"
    /**/    << endl;
    return false;
  }
  // We know at this point that our wanted object has not
  // been read or created yet.
  unique_ptr<EDProduct> edp;
  if (wanted_wrapper_type == secondary_wrapper_type_) {
    /**/cout
    /**/    << "we want the secondary wrapper type"
    /**/    << endl;
    if (Group::uniqueProduct() == nullptr) {
      // Our partner needs to be read or
      // demand produced first.
      /**/cout
      /**/    << "produced object not read yet, trying to obtain it"
      /**/    << endl;
      edp = obtainDesiredProduct(fillOnDemand, producedWrapperType());
      if (edp.get()) {
        /**/cout
        /**/    << "produced object read"
        /**/    << endl
        /**/    << "calling setProduct to set product_ to: "
        /**/    << edp.get()
        /**/    << endl;
        setProduct(move(edp));
      }
      else {
        /**/cout
        /**/    << "failed to get the produced object"
        /**/    << endl;
      }
    }
    if (Group::uniqueProduct() != nullptr) {
      // Our partner has already been read, so call its
      // makePartner function to get what we want.
      /**/cout
      /**/    << "we have the produced product, now calling"
      /**/    << endl
      /**/    << "its makeParner to get what we want"
      /**/    << endl;
      edp = Group::uniqueProduct()->makePartner(wanted_wrapper_type.typeInfo());
      if (edp.get() != nullptr) {
        /**/cout
        /**/    << "setting secondaryProduct_ to: "
        /**/    << edp.get()
        /**/    << endl;
        secondaryProduct_ = move(edp);
      }
      else {
        /**/cout
        /**/    << "makePartner failed to make what we want"
        /**/    << endl;
      }
    }
  }
  else {
    // We want the produced type.
    /**/cout
    /**/    << "we want the produced wrapper type"
    /**/    << endl;
    /**/cout
    /**/    << "produced object not read yet, trying to obtain it"
    /**/    << endl;
    edp = obtainDesiredProduct(fillOnDemand, producedWrapperType());
    if (edp.get()) {
      /**/cout
      /**/    << "produced wrapper type product found"
      /**/    << endl
      /**/    << "calling setProduct to set product_ to: "
      /**/    << edp.get()
      /**/    << endl;
      setProduct(move(edp));
    }
    else {
      /**/cout
      /**/    << "failed to get the produced object"
      /**/    << endl;
    }
  }
  bool retval = false;
  if (uniqueProduct(wanted_wrapper_type) != nullptr) {
    retval = true;
  }
  /**/cout
  /**/    << "returning: "
  /**/    << retval
  /**/    << endl;
  /**/cout
  /**/    << "-----> End   AssnsGroup::resolveProductIfAvailable(...)"
  /**/    << endl;
  return retval;
}

#if 0
unique_ptr<EDProduct>
AssnsGroup::
maybeObtainProductFromPartner(TypeID const& wanted_wrapper_type) const
{
  // If the partner object has already been read,
  // then called its makePartner function to get
  // what we want.  Otherwise return a nullptr.
  //cout
  //    << "-----> Begin AssnsGroup::maybeObtainProductFromPartner(TypeID const&)"
  //    << endl
  //    << "wanted_wrapper_type: "
  //    << cet::demangle_symbol(wanted_wrapper_type.name())
  //    << endl;
  TypeID const& other_wrapper_type = (wanted_wrapper_type == secondary_wrapper_type_) ?  producedWrapperType() : secondary_wrapper_type_;
  //cout
  //    << "other_wrapper_type:  "
  //    << cet::demangle_symbol(other_wrapper_type.name())
  //    << endl;
  unique_ptr<EDProduct> retval;
  if (uniqueProduct(other_wrapper_type)) {
    //cout
    //    << "calling "
    //    << cet::demangle_symbol(other_wrapper_type.name())
    //    << "->makePartner("
    //    << cet::demangle_symbol(wanted_wrapper_type.name())
    //    << ")"
    //    << endl;
    retval = uniqueProduct(other_wrapper_type)->makePartner(wanted_wrapper_type.typeInfo());
    //cout
    //    << "which returned: "
    //    << retval.get()
    //    << endl;
  }
  //cout 
  //    << "returning: "
  //    << retval.get()
  //    << endl;
  //cout
  //    << "-----> End   AssnsGroup::maybeObtainProductFromPartner(TypeID const&)"
  //    << endl;
  return retval;
}
#endif // 0

} // namespace art

