#ifndef art_Framework_Core_ConsumesRecorder_h
#define art_Framework_Core_ConsumesRecorder_h

//---------------------------------------------------------------------------
// ConsumesRecorder: Records which products are intended to be retrieved.
//---------------------------------------------------------------------------

#include "art/Framework/Principal/ProductInfo.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Utilities/InputTag.h"
#include "canvas/Utilities/TypeID.h"

namespace fhicl {
  class ParameterSet;
}

namespace art {

  class ModuleDescription;

  class ConsumesRecorder {
    struct InvalidTag{};
  public:

    explicit ConsumesRecorder() = default;

    static ConsumesRecorder& invalid() {
      static ConsumesRecorder invalidRec{InvalidTag{}};
      return invalidRec;
    }

    // Once all of the 'consumes(Many)' calls have been made for this
    // recorder, the consumables are sorted, and the configuration is
    // retrieved to specify the desired behavior in the case of a
    // missing consumes clause.
    void prepareForJob(fhicl::ParameterSet const& pset);

    // Not sure what the return type of 'consumes' should be yet.
    template <typename, BranchType>
    void consumes(InputTag const&);

    template <typename, BranchType>
    void consumesMany();

    void validateConsumedProduct(BranchType const bt, ProductInfo const& pi);

    void showMissingConsumes(ModuleDescription const& md) const;

  private:

    explicit ConsumesRecorder(InvalidTag) : trackConsumes_{false} {}

    bool trackConsumes_{true};
    bool requireConsumes_{false};
    ConsumableProducts consumables_{};
    ConsumableProductSets missingConsumes_{};
  };
}

//===========================================================================
template <typename T, art::BranchType BT>
void art::ConsumesRecorder::consumes(InputTag const& it)
{
  if (!trackConsumes_) return;

  consumables_[BT].emplace_back(TypeID{typeid(T)},
                                it.label(),
                                it.instance(),
                                it.process());
}

template <typename T, art::BranchType BT>
void art::ConsumesRecorder::consumesMany()
{
  if (!trackConsumes_) return;

  consumables_[BT].emplace_back(TypeID{typeid(T)});
}

#endif /* art_Framework_Core_ConsumesRecorder_h */

// Local Variables:
// mode: c++
// End:
