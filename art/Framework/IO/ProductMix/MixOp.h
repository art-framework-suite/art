#ifndef art_Framework_IO_ProductMix_MixOp_h
#define art_Framework_IO_ProductMix_MixOp_h
// vim: set sw=2 expandtab :

// Template encapsulating all the attributes and functionality of a
// product mixing operation.

#include "art/Framework/Core/InputSourceMutex.h"
#include "art/Framework/IO/ProductMix/MixOpBase.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Utilities/Globals.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/Compatibility/BranchIDList.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/canonicalProductName.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/InputTag.h"
#include "cetlib/exempt_ptr.h"

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>

namespace art {

  template <typename Prod, typename OProd>
  class MixOp : public MixOpBase {
  public:
    template <typename FUNC>
    MixOp(std::string const& moduleLabel,
          InputTag const& inputTag,
          std::string const& outputInstanceLabel,
          FUNC mixFunc,
          bool outputProduct,
          bool compactMissingProducts,
          BranchType bt);

    InputTag const&
    inputTag() const override
    {
      return inputTag_;
    }
    TypeID
    inputType() const override
    {
      return inputType_;
    }
    EDProduct const*
    newIncomingWrappedProduct() const override
    {
      return new Wrapper<Prod>{};
    }
    ProductID incomingProductID() const override;
    ProductID outgoingProductID() const override;
    BranchType branchType() const override;

  private:
    void mixAndPut(Event& e,
                   SpecProdList const& inProducts,
                   PtrRemapper const& remap) const override;
    void setIncomingProductID(ProductID) override;

    InputTag const inputTag_;
    std::string const outputInstanceLabel_;
    TypeID const inputType_;
    MixFunc<Prod, OProd> const mixFunc_;
    std::string const processName_;
    std::string const moduleLabel_;
    bool const outputProduct_;
    bool const compactMissingProducts_;
    BranchType const branchType_;
    ProductID incomingProductID_{ProductID::invalid()};
  };

  template <typename Prod, typename OProd>
  template <typename FUNC>
  MixOp<Prod, OProd>::MixOp(std::string const& moduleLabel,
                            InputTag const& inputTag,
                            std::string const& outputInstanceLabel,
                            FUNC mixFunc,
                            bool const outputProduct,
                            bool const compactMissingProducts,
                            BranchType const bt)
    : inputTag_{inputTag}
    , outputInstanceLabel_{outputInstanceLabel}
    , inputType_{typeid(Prod)}
    , mixFunc_{mixFunc}
    , processName_{Globals::instance()->processName()}
    , moduleLabel_{moduleLabel}
    , outputProduct_{outputProduct}
    , compactMissingProducts_{compactMissingProducts}
    , branchType_{bt}
  {}

  template <typename Prod, typename OProd>
  void
  MixOp<Prod, OProd>::mixAndPut(Event& e,
                                SpecProdList const& inProducts,
                                PtrRemapper const& remap) const
  {
    std::vector<Prod const*> inConverted;
    inConverted.reserve(inProducts.size());
    try {
      for (auto const& ep : inProducts) {
        auto const prod =
          std::dynamic_pointer_cast<Wrapper<Prod> const>(ep)->product();
        if (prod || !compactMissingProducts_) {
          inConverted.emplace_back(prod);
        }
      }
    }
    catch (std::bad_cast const&) {
      throw Exception(errors::DataCorruption)
        << "Unable to obtain correctly-typed product from wrapper.\n";
    }

    auto rProd = std::make_unique<OProd>();
    // False means don't want this in the event.
    if (mixFunc_(inConverted, *rProd, remap)) {
      if (!outputProduct_) {
        throw Exception(errors::LogicError)
          << "Returned true (output product to be put in event) from a mix "
             "function\n"
          << "declared with outputProduct=false.\n";
      }
      e.put(move(rProd), outputInstanceLabel_);
    }
  }

  template <typename Prod, typename OProd>
  void
  MixOp<Prod, OProd>::setIncomingProductID(ProductID const prodID)
  {
    incomingProductID_ = prodID;
  }

  template <typename Prod, typename OProd>
  ProductID
  MixOp<Prod, OProd>::incomingProductID() const
  {
    return incomingProductID_;
  }

  template <typename Prod, typename OProd>
  ProductID
  MixOp<Prod, OProd>::outgoingProductID() const
  {
    ProductID result;
    if (outputProduct_) {
      TypeID const outputType{typeid(OProd)};
      // Note: Outgoing product must be InEvent.
      auto const productName =
        canonicalProductName(outputType.friendlyClassName(),
                             moduleLabel_,
                             outputInstanceLabel_,
                             processName_);
      result = ProductID{productName};
    }
    return result;
  }

  template <typename Prod, typename OProd>
  inline BranchType
  MixOp<Prod, OProd>::branchType() const
  {
    return branchType_;
  }

} // namespace art

#endif /* art_Framework_IO_ProductMix_MixOp_h */

// Local Variables:
// mode: c++
// End:
