#include "art/Framework/Core/MasterProductRegistry.h"

#include "art/Persistency/Provenance/BranchKey.h"
#include "art/Persistency/Provenance/ReflexTools.h"
#include "art/Utilities/TypeID.h"
#include "art/Utilities/WrappedClassName.h"
#include "cetlib/exception.h"

#include <ostream>
#include <sstream>

namespace {
  void checkDicts(art::BranchDescription const &productDesc) {
    if (productDesc.transient()) { // FIXME: make this go away soon.
      art::checkDictionaries(productDesc.fullClassName(), true);
    } else {
      art::checkDictionaries(art::wrappedClassName(productDesc.fullClassName()), false);
    }
  }
}

std::ostream &
operator<<(std::ostream &os, art::MasterProductRegistry const &mpr) {
  mpr.print(os);
  return os;
}

std::vector<art::BranchDescription const *>
art::MasterProductRegistry::allBranchDescriptions() const {
  std::vector<BranchDescription const *> result;
  result.reserve(productList_.size());
  for (ProductList::const_iterator
         i = productList_.begin(),
         e = productList_.end();
       i != e; ++i) {
    result.push_back(&i->second);
  }
  return result;
}

std::vector<std::string>
art::MasterProductRegistry::allBranchNames() const {
  std::vector<std::string> result;
  result.reserve(productList_.size());
  for (ProductList::const_iterator
         i = productList_.begin(),
         e = productList_.end();
       i != e; ++i) {
    result.push_back(i->second.branchName());
  }
  return result;
}

bool
art::MasterProductRegistry::anyProducts(BranchType branchType) const {
  throwIfNotFrozen();
  for (ProductList::const_iterator
         it = productList_.begin(),
         itEnd = productList_.end();
       it != itEnd;
       ++it) {
    if (it->second.branchType() == branchType) {
      return true;
    }
  }
  return false;
}

void
art::MasterProductRegistry::print(std::ostream &os) const {
  for (ProductList::const_iterator
         i = productList_.begin(),
         e = productList_.end();
       i != e;
       ++i) {
    os << i->second << "\n-----\n";
  }
}

void
art::MasterProductRegistry::addProduct(BranchDescription  &productDesc) {
  assert(productDesc.produced());
  throwIfFrozen();
  productDesc.init();
  checkDicts(productDesc);
  if (!productList_.insert(std::make_pair(BranchKey(productDesc), productDesc)).second) {
    throw art::Exception(errors::Configuration)
      << "The process name "
      << productDesc.processName()
      << " was previously used on these products.\n"
      << "Please modify the configuration file to use a "
      << "distinct process name.\n";
  }
}

void
art::MasterProductRegistry::updateFromInput(ProductList const &other) {
  for (ProductList::const_iterator
         it = other.begin(),
         itEnd = other.end();
       it != itEnd;
       ++it) {
    copyProduct(it->second);
  }
}

void
art::MasterProductRegistry::updateFromInput(std::vector<BranchDescription> const &other) {
  for (std::vector<BranchDescription>::const_iterator
         it = other.begin(),
         itEnd = other.end();
       it != itEnd;
       ++it) {
    copyProduct(*it);
  }

}

std::string
art::MasterProductRegistry::merge(ProductList const &other,
                                          std::string const &fileName,
                                          BranchDescription::MatchMode m) {
  std::ostringstream differences;

  ProductList::iterator pli = productList_.begin();
  ProductList::iterator ple = productList_.end();
  ProductList::const_iterator oi = other.begin();
  ProductList::const_iterator oe = other.end();

  // Loop over entries in the main product registry.
  while (pli != ple || oi != oe) {
    if (pli != ple && pli->second.produced()) {
      // Ignore branches pliust produced (oi.oe. not in input file).
      ++pli;
    } else if (pli == ple || (oi != oe && oi->first < pli->first)) {
      if (oi->second.present()) {
        differences << "Branch '"
                    << oi->second.branchName()
                    << "' is in file '"
                    << fileName
                    << "'\n";
        differences << "    but not in previous files.\n";
      } else {
        productList_.insert(*oi);
      }
      ++oi;
    } else if (oi == oe || (pli != ple && pli->first < oi->first)) {
      // Allow branch to be missing in new file.
      ++pli;
    } else {
      std::string diffs = match(pli->second, oi->second, fileName, m);
      if (diffs.empty()) {
        if (m == BranchDescription::Permissive) pli->second.merge(oi->second);
      } else {
        differences << diffs;
      }
      ++oi;
      ++pli;
    }
  }
  processFrozenProductList();
  return differences.str();
}

void
art::MasterProductRegistry::setFrozen() {
    checkAllDictionaries();
    if (frozen_) return;
    frozen_ = true;
    processFrozenProductList();
}

void
art::MasterProductRegistry::copyProduct(BranchDescription const &productDesc) {
  assert(!productDesc.produced());
  throwIfFrozen();
  productDesc.init();
  BranchKey k = BranchKey(productDesc);
  ProductList::iterator iter = productList_.find(k);
  if (iter == productList_.end()) {
    productList_.insert(std::make_pair(k, productDesc));
  } else {
    assert(combinable(iter->second, productDesc));
    iter->second.merge(productDesc);
  }
}

void
art::MasterProductRegistry::fillElementLookup(Reflex::Type const &type,
                                                      BranchID const &id,
                                                      BranchKey const &bk) {
  TypeID typeID(type.TypeInfo());
  std::string friendlyClassName = typeID.friendlyClassName();
  ProcessLookup& processLookup = elementLookup_[friendlyClassName];
  std::vector<BranchID>& vint = processLookup[bk.processName_];
  vint.push_back(id);
}

void
art::MasterProductRegistry::throwIfNotFrozen() const {
  if (!frozen_) {
    throw cet::exception("ProductRegistry", "throwIfNotFrozen")
      << "Cannot read the ProductRegistry because it is not yet frozen.\n";
  }
}

void
art::MasterProductRegistry::throwIfFrozen() const {
  if (frozen_) {
    throw cet::exception("ProductRegistry", "throwIfFrozen")
      << "Cannot modify the ProductRegistry because it is frozen.\n";
  }
}

void
art::MasterProductRegistry::processFrozenProductList() {
  productLookup_.clear();
  elementLookup_.clear();
  for (ProductList::const_iterator
         i = productList_.begin(),
         e = productList_.end();
       i != e;
       ++i) {
    if (i->second.produced()) {
      productProduced_[i->second.branchType()] = true;
    }

    ProcessLookup &processLookup = productLookup_[i->first.friendlyClassName_];
    std::vector<BranchID> &vint = processLookup[i->first.processName_];
    vint.push_back(i->second.branchID());

    Reflex::Type type(Reflex::Type::ByName(i->second.className()));
    if (type) {
      // Here we look in the object named "type" for a typedef named
      // "value_type" and get the Reflex::Type for it.  Then check to
      // ensure the Reflex dictionary is defined for this value_type.  I
      // do not throw an exception here if the check fails because there
      // are known cases where the dictionary does not exist and we do
      // not need to support those cases.
      Reflex::Type valueType;
      if (value_type_of(type, valueType) && static_cast<bool>(valueType)) {
        // FIXME: Why is there an explicit static_cast above? Reflex type foo?
        fillElementLookup(valueType, i->second.branchID(), i->first);

        // Repeat this for all public base classes of the value_type
        std::vector<Reflex::Type> baseTypes;
        public_base_classes(valueType, baseTypes);

        for (std::vector<Reflex::Type>::iterator
               iter = baseTypes.begin(),
               iend = baseTypes.end();
             iter != iend;
             ++iter) {
          fillElementLookup(*iter, i->second.branchID(), i->first);
        }
      }
    }
  }
}
