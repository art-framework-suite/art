#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ProductList.h"
#include "art/Persistency/Provenance/BranchIDListHelper.h"
#include "art/Persistency/Provenance/BranchIDListRegistry.h"
#include "art/Persistency/Provenance/BranchKey.h"
#include "art/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include <iomanip>
#include <sstream>
#include <utility>

namespace {

  using BranchIDList = art::BranchIDListRegistry::collection_type::value_type;

  std::size_t width(BranchIDList const& v)
  {
    std::size_t w(0);
    cet::for_all(v,[&w](auto entry){ w=std::max(w,std::to_string(entry).size()); } );
    return w;
  }

  std::string
  mismatch_msg(BranchIDList const & i, BranchIDList const & j)
  {
    auto iIter = i.cbegin();
    auto jIter = j.cbegin();
    const auto iEnd = i.cend();
    const auto jEnd = j.cend();
    std::size_t const iW = std::max(width(i),std::string("Previous File").size());
    std::size_t const jW = std::max(width(j),std::string("File to merge").size());
    std::ostringstream msg;
    msg << " Previous File    File to merge\n";
    msg << " ==============================\n";
    for (; iIter != iEnd || jIter != jEnd ;) {
      std::string imsg, jmsg;
      if ( iIter != iEnd ) imsg = std::to_string(*iIter++);
      if ( jIter != jEnd ) jmsg = std::to_string(*jIter++);
      msg << " "
          << std::setw(iW) << std::left << imsg
          << "    "
          << std::setw(jW) << std::left << jmsg
          << '\n';
    }

    return msg.str();
  }

}

namespace art {

  void
  BranchIDListHelper:: updateFromInput(BranchIDLists const& bidlists, std::string const& fileName)
  {
    typedef BranchIDListRegistry::const_iterator iter;
    BranchIDListRegistry& breg = *BranchIDListRegistry::instance();
    BranchIDListRegistry::collection_type& bdata = breg.data();
    iter j = bidlists.begin(), jEnd = bidlists.end();
    for(iter i = bdata.begin(), iEnd = bdata.end(); j != jEnd && i != iEnd; ++j, ++i) {
      if (*i != *j) {
        std::string const errmsg = mismatch_msg( *i, *j );
        throw art::Exception(errors::UnimplementedFeature)
          << "Cannot merge file '" << fileName << "' due to a branch mismatch.\n\n"
          << errmsg
          << "\n\n"
          << "The BranchIDs above correspond to products\n"
          << "that were created whenever the current input files\n"
          << "were produced.  The lists above must be identical.\n"
          << "To determine which products these BranchIDs correspond to,\n"
          << "rerun the process that produced the input files,\n"
          << "enabling full debug output for the message service.\n"
          << "Then 'grep' the log file for messages with 'BranchID'."
          << "\n\n"
          << "Contact the framework group for assistance.\n";
      }
    }
    for (; j != jEnd; ++j) {
      breg.insertMapped(*j);
    }
  }

  void
  BranchIDListHelper::updateRegistries(MasterProductRegistry const& preg)
  {
    BranchIDList bidlist;
    // Add entries for current process for ProductID to BranchID mapping.
    for (ProductList::const_iterator
           it = preg.productList().begin(),
           itEnd = preg.productList().end();
         it != itEnd;
         ++it) {
      if (it->second.produced()) {
        if (it->second.branchType() == InEvent) {
          bidlist.push_back(it->second.branchID().id());
        }
      }
    }
    BranchIDListRegistry& breg = *BranchIDListRegistry::instance();
    breg.insertMapped(bidlist);

    // Add entries to aid BranchID to ProductID mapping
    BranchIDToIndexMap& branchIDToIndexMap = breg.extra().branchIDToIndexMap_;
    for (BranchIDLists::const_iterator it = breg.data().begin(), itEnd = breg.data().end(); it != itEnd; ++it) {
      BranchListIndex blix = it - breg.data().begin();
      for (BranchIDList::const_iterator i = it->begin(), iEnd = it->end(); i != iEnd; ++i) {
        ProductIndex pix = i - it->begin();
        branchIDToIndexMap.insert(std::make_pair(BranchID(*i), std::make_pair(blix, pix)));
      }
    }
  }

  void
  BranchIDListHelper::clearRegistries()
  {
    BranchIDListRegistry& breg = *BranchIDListRegistry::instance();
    breg.data().clear();
    breg.extra().branchIDToIndexMap_.clear();
  }
}
