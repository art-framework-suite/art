#include "art/Persistency/Provenance/BranchEntryDescription.h"

#include "art/Persistency/Provenance/EntryDescriptionRegistry.h"
#include "cetlib/container_algorithms.h"
#include <ostream>


using namespace cet;
using namespace std;


namespace art {

  BranchEntryDescription::BranchEntryDescription() :
    productID_(),
    parents_(),
    cid_(),
    status_(Success),
    isPresent_(false),
    moduleDescriptionID_()
  { }

  BranchEntryDescription::BranchEntryDescription(ProductID const& pid,
         BranchEntryDescription::CreatorStatus const& status) :
    productID_(pid),
    parents_(),
    cid_(),
    status_(status),
    isPresent_(status == Success),
    moduleDescriptionID_()
  { }

  void
  BranchEntryDescription::write(ostream& os) const {
    // This is grossly inadequate, but it is not critical for the
    // first pass.
    os << "Product ID = " << productID_ << '\n';
    os << "CreatorStatus = " << creatorStatus() << '\n';
    os << "Module Description ID = " << moduleDescriptionID() << '\n';
    os << "Is Present = " << isPresent() << endl;
  }

  bool
  operator==(BranchEntryDescription const& a, BranchEntryDescription const& b) {
    return
      a.productID() == b.productID()
      && a.creatorStatus() == b.creatorStatus()
      && a.parents() == b.parents()
      && a.moduleDescriptionID() == b.moduleDescriptionID();
  }

  auto_ptr<EntryDescription>
  BranchEntryDescription::convertToEntryDescription() const {
    auto_ptr<EntryDescription> entryDescription(new EntryDescription);
    entryDescription->parents() = parents();
    entryDescription->moduleDescriptionID() = moduleDescriptionID();
    return entryDescription;
  }

}  // namespace art
