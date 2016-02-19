#include "art/Persistency/Provenance/BranchIDListHelper.h"
// vim: set sw=2:

#include "art/Persistency/Provenance/BranchIDListRegistry.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"

#include <iomanip>
#include <sstream>
#include <utility>

namespace {

  using BranchIDList = art::BranchIDListRegistry::collection_type::value_type;

  std::size_t width(BranchIDList const& v)
  {
    std::size_t w{};
    cet::for_all(v, [&w](auto entry){w = std::max(w, std::to_string(entry).size());} );
    return w;
  }

  std::string
  mismatch_msg(BranchIDList const& i, BranchIDList const& j)
  {
    std::size_t const iW = std::max(width(i), std::string{"Previous File"}.size());
    std::size_t const jW = std::max(width(j), std::string{"File to merge"}.size());

    std::ostringstream msg;
    msg << "  Previous File    File to merge\n";
    msg << "  ==============================\n";
    for (std::size_t m{}; m != std::max(i.size(), j.size()); ++m) {
      msg << "  "
          << std::setw(iW)
          << std::left
          << (m < i.size() ? std::to_string(i[m]) : "")
          << "    "
          << std::setw(jW)
          << std::left
          << (m < j.size() ? std::to_string(j[m]) : "")
          << '\n';
    }
    return msg.str();
  }

  auto first_new_BranchIDList(art::BranchIDLists const& ref,
                              art::BranchIDLists const& test,
                              std::string const& fileName)
  {
    std::string err_msg;
    std::size_t i{};
    for ( ; i != std::min(ref.size(), test.size()); ++i ) {
      if ( ref[i] != test[i] ) {
        err_msg += "Process " + std::to_string(i+1) + ":\n\n";
        err_msg += mismatch_msg(ref[i], test[i]);
        err_msg += "\n\n";
      }
    }

    return
      err_msg.empty() ?
      std::next(test.begin(),i) : // will not exceed test.end()
      throw art::Exception(art::errors::MismatchedInputFiles)
      << "Cannot merge file '"
      << fileName
      << "' due to inconsistent process histories:\n\n"
      << err_msg
      << "The BranchIDs above correspond to products that were\n"
      << "created whenever the current input files were produced.\n"
      << "\nThe lists above must be identical per process.\n\n"
      << "To determine which products these BranchIDs correspond to,\n"
      << "rerun the corresponding processes that produced the\n"
      << "input files, enabling full debug output for the message\n"
      << "service. Then 'grep' the log file for messages with 'BranchID'."
      << "\n\n"
      << "Contact the framework group for assistance.\n";
  }

} // unnamed namespace

// Called on primary file open.  Verify that the new file produced the
// same Event-level products for each process in the process history
// it has in common with the previous files, and allow it to add more
// process history onto the end.
void
art::
BranchIDListHelper::
updateFromInput(BranchIDLists const & file_bidlists, std::string const& fileName)
{
  auto& breg = *BranchIDListRegistry::instance();
  auto& reg_bidlists = breg.data();

  std::for_each( first_new_BranchIDList(reg_bidlists, file_bidlists, fileName),
                 file_bidlists.cend(),
                 [&breg](auto const& new_list) {
                   breg.insertMapped( new_list );
                 } );

  generate_branchIDToIndexMap();
}

// Last thing done by the EventProcessor constructor:
//
//   Insert an entry into the BranchIDListRegistry (which is either
//   empty or read from the first input file) for the products
//   produced by the current process.
//
void
art::
BranchIDListHelper::
updateRegistries(MasterProductRegistry const& mpr)
{
  // Accumulate a list of branch id's produced by the current process
  // for use by ProductID <--> BranchID mapping.
  BranchIDList bidlist;
  for (auto const& val : mpr.productList()) {
    if (val.second.produced() && (val.second.branchType() == InEvent)) {
      bidlist.push_back(val.second.branchID().id());
    }
  }
  // In what follows, if mpr.productList() is empty, and empty bidlist
  // is inserted onto the BranchIDListRegistry.  This ensures that no
  // input files with *additional* BranchIDList elements can be
  // concatenated on input with the ones before it.
  //
  // This restriction can be lifted in the future by simply placing an
  // 'if ( !bidlist.empty() )' condition in front of the following
  // statement.  Doing so, however, could cause a breaking change in
  // the following scenario:
  //
  // Suppose somebody has the following workflow that produce the
  // following series of BranchIDLists:
  //
  //    BranchIDLists from    |  Old art    |  New art (with 'if' condition)
  //    ====================================================================
  //    From input file       | [b11, b12]  | [b11, b12]
  //    Drop products         | []          | — no entry -
  //    Add new product       | [b31]       | [b31]
  //    Output file #BIDLists | 3           | 2
  //
  // Then files produced from old art will be incompatible with files
  // produced with new art even though the configuration for each are
  // the same.
  BranchIDListRegistry::instance()->insertMapped(bidlist);

  generate_branchIDToIndexMap();
}

void
art::
BranchIDListHelper::
clearRegistries()
{
  auto& breg = *BranchIDListRegistry::instance();
  breg.data().clear();
  breg.extra().branchIDToIndexMap_.clear();
}

void
art::
BranchIDListHelper::
generate_branchIDToIndexMap()
{
  auto& breg = *BranchIDListRegistry::instance();
  auto& branchIDToIndexMap = breg.extra().branchIDToIndexMap_;
  auto& bidlists = breg.data();
  for(std::size_t n=0, n_max = bidlists.size(); n != n_max; ++n ) {
    for (std::size_t m=0, m_max = bidlists[n].size(); m != m_max; ++m ) {
      branchIDToIndexMap.emplace( BranchID{bidlists[n][m]}, IndexPair{n,m});
    }
  }
}
