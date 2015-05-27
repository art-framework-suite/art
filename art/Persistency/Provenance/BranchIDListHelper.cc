#include "art/Persistency/Provenance/BranchIDListHelper.h"
// vim: set sw=2:

#include "art/Persistency/Provenance/BranchIDListRegistry.h"
#include "art/Persistency/Provenance/BranchKey.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ProductList.h"
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
  cet::for_all(v, [&w](auto entry) {
    w = std::max(w, std::to_string(entry).size());
  });
  return w;
}

std::string
mismatch_msg(BranchIDList const& i, BranchIDList const& j)
{
  auto iIter = i.cbegin();
  auto jIter = j.cbegin();
  const auto iEnd = i.cend();
  const auto jEnd = j.cend();
  std::size_t const iW = std::max(width(i),
                                  std::string("Previous File").size());
  std::size_t const jW = std::max(width(j),
                                  std::string("File to merge").size());
  std::ostringstream msg;
  msg << " Previous File    File to merge\n";
  msg << " ==============================\n";
  while ((iIter != iEnd) || (jIter != jEnd)) {
    std::string imsg;
    std::string jmsg;
    if (iIter != iEnd) {
      imsg = std::to_string(*iIter++);
    }
    if (jIter != jEnd) {
      jmsg = std::to_string(*jIter++);
    }
    msg << " "
        << std::setw(iW)
        << std::left
        << imsg
        << "    "
        << std::setw(jW)
        << std::left
        << jmsg
        << '\n';
  }
  return msg.str();
}

} // unnamed namespace

namespace art {

// Called on primary file open.
// Verify that the new file produced the same products for each process
// in the process history it has in common with the previous files, and
// allow it to add more process history onto the end.
void
BranchIDListHelper::
updateFromInput(BranchIDLists const& bidlists, std::string const& fileName)
{
  auto& breg = *BranchIDListRegistry::instance();
  auto& bdata = breg.data();
  auto J = bidlists.cbegin();
  auto JE = bidlists.cend();
  for (auto I = bdata.cbegin(), IE = bdata.cend(); (J != JE) && (I != IE);
      ++J, ++I) {
    // FIXME: Note that this check only compares lists of products
    // FIXME: produced in processes, it does not compare the process
    // FIXME: names.  This possibly allows files containing data from
    // FIXME: two incompatible processes to be mixed!
    if (*I != *J) {
      std::string const errmsg = mismatch_msg(*I, *J);
      throw art::Exception(errors::UnimplementedFeature)
          << "Cannot merge file '"
          << fileName
          << "' due to a branch mismatch.\n\n"
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
  for (; J != JE; ++J) {
    breg.insertMapped(*J);
  }
  // FIXME: We need to generate the reverse mappings here!
}

// Last thing done by the EventProcessor constructor.
// Insert an entry into the BranchIDListRegistry (either empty or read
// from the first input file) for the products produced by the current
// process.  Also generate a full reverse mapping.
void
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
  auto& breg = *BranchIDListRegistry::instance();
  // Put accumulated list into registry.
  breg.insertMapped(bidlist);
  // Now generate the reverse mapping.
  auto& branchIDToIndexMap = breg.extra().branchIDToIndexMap_;
  for (auto I = breg.data().cbegin(), E = breg.data().cend(); I != E; ++I) {
    BranchListIndex blix = I - breg.data().begin();
    for (auto J = I->cbegin(), JE = I->cend(); J != JE; ++J) {
      ProductIndex pix = J - I->begin();
      branchIDToIndexMap.emplace(BranchID(*J), std::make_pair(blix, pix));
    }
  }
}

void
BranchIDListHelper::
clearRegistries()
{
  auto& breg = *BranchIDListRegistry::instance();
  breg.data().clear();
  breg.extra().branchIDToIndexMap_.clear();
}

} // namespace art

