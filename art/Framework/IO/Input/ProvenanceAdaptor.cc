// ======================================================================
//
// ProvenanceAdaptor.cc
//
// ======================================================================


#include "art/Framework/IO/Input/ProvenanceAdaptor.h"

#include "art/Persistency/Provenance/ProcessConfiguration.h"
#include "art/Persistency/Provenance/ProcessHistory.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "cetlib/container_algorithms.h"
#include <cassert>
#include <memory>
#include <set>
#include <utility>


using namespace cet;
using namespace std;


namespace art {

  //------------------------------------------------------------
  // Class ProvenanceAdaptor: adapts old provenance (fileFormatVersion_.value_ < 11) to new provenance.
  namespace {
    typedef vector<string> OneHistory;
    typedef set<OneHistory> Histories;
    typedef pair<string, BranchID> Product;
    typedef vector<Product> OrderedProducts;

    struct Sorter {
      explicit Sorter(Histories const& histories) : histories_(histories) {}
      bool operator()(Product const& a, Product const& b) const;
      Histories const histories_;
    };

    bool Sorter::operator()(Product const& a, Product const& b) const {
      assert (a != b);
      if (a.first == b.first) return false;
      bool mayBeTrue = false;
      for (Histories::const_iterator it = histories_.begin(), itEnd = histories_.end(); it != itEnd; ++it) {
        OneHistory::const_iterator itA = find_in_all(*it, a.first);
        if (itA == it->end()) continue;
        OneHistory::const_iterator itB = find_in_all(*it, b.first);
        if (itB == it->end()) continue;
        assert (itA != itB);
        if (itB < itA) {
          // process b precedes process a;
          return false;
        }
        // process a precedes process b;
        mayBeTrue = true;
      }
      return mayBeTrue;
    }
  }

  ProvenanceAdaptor::ProvenanceAdaptor(
             ProductRegistry const& productRegistry,
             ProcessHistoryMap const& pHistMap,
             ParameterSetMap const& psetMap,
             ModuleDescriptionMap const&  mdMap) :
                productRegistry_(productRegistry),
                branchIDLists_(),
                branchListIndexes_() {

    OrderedProducts orderedProducts;
    set<string> processNamesThatProduced;
    ProductRegistry::ProductList const& prodList = productRegistry.productList();
    for (ProductRegistry::ProductList::const_iterator it = prodList.begin(), itEnd = prodList.end(); it != itEnd; ++it) {
      if (it->second.branchType() == InEvent) {
        it->second.init();
        processNamesThatProduced.insert(it->second.processName());
        orderedProducts.push_back(make_pair(it->second.processName(), it->second.branchID()));
      }
    }
    assert (!orderedProducts.empty());
    Histories processHistories;
    size_t max = 0;
    for(ProcessHistoryMap::const_iterator it = pHistMap.begin(), itEnd = pHistMap.end(); it != itEnd; ++it) {
      ProcessHistory const& pHist = it->second;
      OneHistory processHistory;
      for(ProcessHistory::const_iterator i = pHist.begin(), iEnd = pHist.end(); i != iEnd; ++i) {
        if (processNamesThatProduced.find(i->processName()) != processNamesThatProduced.end()) {
          processHistory.push_back(i->processName());
        }
      }
      max = (processHistory.size() > max ? processHistory.size() : max);
      assert(max <= processNamesThatProduced.size());
      if (processHistory.size() > 1) {
        processHistories.insert(processHistory);
      }
    }
    stable_sort_all(orderedProducts, Sorter(processHistories));

    auto_ptr<BranchIDLists> pv(new BranchIDLists);
    auto_ptr<BranchIDList> p(new BranchIDList);
    string processName;
    BranchListIndex blix = 0;
    for (OrderedProducts::const_iterator it = orderedProducts.begin(), itEnd = orderedProducts.end(); it != itEnd; ++it) {
      bool newvector = it->first != processName && !processName.empty();
      if (newvector) {
        pv->push_back(*p);
        branchListIndexes_.push_back(blix);
        ++blix;
        processName = it->first;
        p.reset(new BranchIDList);
      }
      p->push_back(it->second.id());
    }
    pv->push_back(*p);
    branchListIndexes_.push_back(blix);
    branchIDLists_.reset(pv.release());
  }

  boost::shared_ptr<BranchIDLists const>
  ProvenanceAdaptor::branchIDLists() const {
    return branchIDLists_;
  }

  void
  ProvenanceAdaptor::branchListIndexes(BranchListIndexes & indexes)  const {
    indexes = branchListIndexes_;
  }

}  // namespace art
