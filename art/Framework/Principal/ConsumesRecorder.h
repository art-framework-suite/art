#ifndef art_Framework_Principal_ConsumesRecorder_h
#define art_Framework_Principal_ConsumesRecorder_h

//---------------------------------------------------------------------------
// ConsumesRecorder: Records which products are intended to be retrieved.
//---------------------------------------------------------------------------

#include "art/Framework/Principal/ProductInfo.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductToken.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Utilities/InputTag.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/exempt_ptr.h"

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

    // After the modules are constructed, their ModuleDescription
    // values are assigned.  We receive the values of that assignment.
    void setModuleDescription(ModuleDescription const& md);

    // Once all of the 'consumes(Many)' calls have been made for this
    // recorder, the consumables are sorted, and the configuration is
    // retrieved to specify the desired behavior in the case of a
    // missing consumes clause.
    void prepareForJob(fhicl::ParameterSet const& pset);

    // Not sure what the return type of 'consumes' should be yet.
    template <typename T, BranchType>
    ProductToken<T> consumes(InputTag const&);

    template <typename, BranchType>
    void consumesMany();
    void validateConsumedProduct(BranchType const bt, ProductInfo const& pi);
    void showMissingConsumes() const;

  private:

    explicit ConsumesRecorder(InvalidTag) : trackConsumes_{false} {}

    bool trackConsumes_{true};
    bool requireConsumes_{false};
    ConsumableProducts consumables_{};
    ConsumableProductSets missingConsumes_{};
    cet::exempt_ptr<ModuleDescription const> moduleDescription_{nullptr};
  };
}

//===========================================================================
template <typename T, art::BranchType BT>
art::ProductToken<T>
art::ConsumesRecorder::consumes(InputTag const& it)
{
  if (!trackConsumes_) return ProductToken<T>{}; // THIS CAN'T BE RIGHT.

  consumables_[BT].emplace_back(TypeID{typeid(T)},
                                it.label(),
                                it.instance(),
                                it.process());
  return ProductToken<T>{it};
}

template <typename T, art::BranchType BT>
void art::ConsumesRecorder::consumesMany()
{
  if (!trackConsumes_) return;

  consumables_[BT].emplace_back(TypeID{typeid(T)});
}

#endif /* art_Framework_Principal_ConsumesRecorder_h */

// Local Variables:
// mode: c++
// End:
