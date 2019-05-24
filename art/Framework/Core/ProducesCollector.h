#ifndef art_Framework_Core_ProducesCollector_h
#define art_Framework_Core_ProducesCollector_h

#include "art/Persistency/Provenance/detail/branchNameComponentChecking.h"
#include "canvas/Persistency/Common/traits.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/Persistable.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Persistency/Provenance/type_aliases.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"

#include <array>
#include <string>

namespace art {

  class ModuleDescription;

  namespace {

    inline void
    verifyFriendlyClassName(std::string const& fcn)
    {
      std::string errMsg;
      if (!detail::checkFriendlyName(fcn, errMsg)) {
        throw Exception(errors::Configuration)
          << errMsg
          << "In particular, underscores are not permissible anywhere in the "
             "fully-scoped\n"
             "class name, including namespaces.\n";
      }
    }

    inline void
    verifyModuleLabel(std::string const& ml)
    {
      std::string errMsg;
      if (!detail::checkModuleLabel(ml, errMsg)) {
        throw Exception(errors::Configuration) << errMsg;
      }
    }

    inline void
    verifyInstanceName(std::string const& instanceName)
    {
      std::string errMsg;
      if (!detail::checkInstanceName(instanceName, errMsg)) {
        throw Exception(errors::Configuration) << errMsg;
      }
    }

  } // unnamed namespace

  class ProducesCollector {
  public:
    // Record the production of an object of type P, with optional
    // instance name, in the Event (by default), Run, or SubRun.
    template <typename P, BranchType B = InEvent>
    void produces(std::string const& instanceName = {},
                  Persistable const persistable = Persistable::Yes);

    // Record the reconstitution of an object of type P, in either the
    // Run, SubRun, or Event, recording that this object was
    // originally created by a module with label modLabel, and with an
    // optional instance name.
    template <typename P, BranchType B>
    TypeLabel const& reconstitutes(std::string const& modLabel,
                                   std::string const& instanceName = {});

    TypeLabelLookup_t const& expectedProducts(BranchType) const;
    void fillDescriptions(ModuleDescription const& md);

  private:
    TypeLabel const& insertOrThrow(BranchType const bt, TypeLabel const& tl);

    std::array<TypeLabelLookup_t, NumBranchTypes> typeLabelList_{{}};
  };

  inline TypeLabelLookup_t const&
  ProducesCollector::expectedProducts(BranchType const bt) const
  {
    return typeLabelList_[bt];
  }

  template <typename P, art::BranchType B>
  inline void
  ProducesCollector::produces(std::string const& instanceName,
                              Persistable const persistable)
  {
    verifyInstanceName(instanceName);
    TypeID const productType{typeid(P)};
    verifyFriendlyClassName(productType.friendlyClassName());
    bool const isTransient = (persistable == Persistable::No);
    TypeLabel const typeLabel{
      productType, instanceName, SupportsView<P>::value, isTransient};
    insertOrThrow(B, typeLabel);
  }

  template <typename P, BranchType B>
  TypeLabel const&
  ProducesCollector::reconstitutes(std::string const& emulatedModule,
                                   std::string const& instanceName)
  {
    verifyModuleLabel(emulatedModule);
    verifyInstanceName(instanceName);
    TypeID const productType{typeid(P)};
    verifyFriendlyClassName(productType.friendlyClassName());
    TypeLabel const typeLabel{
      productType, instanceName, SupportsView<P>::value, emulatedModule};
    return insertOrThrow(B, typeLabel);
  }

}

#endif
