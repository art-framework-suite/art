#ifndef art_Framework_Core_ConsumerBase_h
#define art_Framework_Core_ConsumerBase_h

//---------------------------------------------------------------------------
// ConsumerBase: The base class of all "modules" that will retrieve products.
//---------------------------------------------------------------------------

#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Utilities/InputTag.h"
#include "canvas/Utilities/TypeID.h"

#include <vector>

namespace art {
  class ConsumerBase {
  public:
    // Not sure what the return type of 'consumes' should be yet.
    template <typename, BranchType = InEvent>
    void consumes(InputTag const&);
  private:
    class ProductInfo {
    public:
      explicit ProductInfo(TypeID const& tid,
                           std::string const& l,
                           std::string const& i,
                           std::string const& pr) :
        typeID_{tid}, label_{l}, instance_{i}, process_{pr}
      {}
    private:
      TypeID typeID_;
      std::string label_;
      std::string instance_;
      std::string process_;
    };
    std::vector<ProductInfo> productsToConsume_{};
  };
}

//===========================================================================
template <typename T, art::BranchType BT>
void art::ConsumerBase::consumes(InputTag const& it)
{
  productsToConsume_.emplace_back(TypeID{typeid(T)},
                                  it.label(),
                                  it.instance(),
                                  it.process());
}


#endif /* art_Framework_Core_ConsumerBase_h */

// Local Variables:
// mode: c++
// End:
