#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "Reflex/Type.h"
#include "art/Persistency/Provenance/BranchKey.h"
#include "art/Persistency/Provenance/ReflexTools.h"
#include "art/Utilities/TypeID.h"
#include "art/Utilities/WrappedClassName.h"
#include "cetlib/exception.h"
#include <cassert>
#include <ostream>
#include <sstream>

namespace {
  void checkDicts(art::BranchDescription const &productDesc) {
    if (productDesc.transient()) { // FIXME: make this go away soon.
      art::checkDictionaries(productDesc.producedClassName(), true);
    } else {
      art::checkDictionaries(art::wrappedClassName(productDesc.producedClassName()), false);
    }
  }
}

std::ostream &
operator<<(std::ostream &os, art::MasterProductRegistry const &mpr) {
  mpr.print(os);
  return os;
}

art::MasterProductRegistry::MasterProductRegistry()
  :
  productList_(),
  frozen_(false),
  productProduced_(),
  productLookup_(),
  elementLookup_()
{
   // FIXME: Use C++2011 initialization when available.
  for (std::array<bool, NumBranchTypes>::size_type i = 0;
       i < productProduced_.size();
       ++i) {
    productProduced_[i] = false;
  }
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
art::MasterProductRegistry::addProduct(std::auto_ptr<BranchDescription> bdp) {
  assert(bdp->produced());
  throwIfFrozen();
  checkDicts(*bdp);
  std::pair<ProductList::iterator, bool> result =
    productList_.insert(std::make_pair(BranchKey(*bdp), BranchDescription()));
  if (result.second) {
    result.first->second.swap(*bdp);
  } else { // Error.
    throw art::Exception(errors::Configuration)
      << "The process name "
      << bdp->processName()
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
    reportFailedDictionaryChecks();
    if (frozen_) return;
    frozen_ = true;
    processFrozenProductList();
}

void
art::MasterProductRegistry::copyProduct(BranchDescription const &productDesc) {
  assert(!productDesc.produced());
  throwIfFrozen();
  checkDicts(productDesc);
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

    Reflex::Type type(Reflex::Type::ByName(i->second.producedClassName()));
    if (type) {
      // Here we look in the object named "type" for a typedef named
      // "value_type" and get the Reflex::Type for it.  Then check to
      // ensure the Reflex dictionary is defined for this value_type.  I
      // do not throw an exception here if the check fails because there
      // are known cases where the dictionary does not exist and we do
      // not need to support those cases.
      Reflex::Type valueType;
      if (value_type_of(type, valueType) && valueType) {
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
