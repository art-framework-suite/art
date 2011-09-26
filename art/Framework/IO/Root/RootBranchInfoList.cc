#include "art/Framework/IO/Root/RootBranchInfoList.h"

#include "art/Utilities/Exception.h"
#include "art/Utilities/InputTag.h"

#include "cpp0x/regex"

#include "TIterator.h"
#include "TObjArray.h"

art::RootBranchInfoList::RootBranchInfoList()
  :
  data_()
{}

art::RootBranchInfoList::RootBranchInfoList(TTree *tree)
  :
  data_()
{
  reset(tree);
}

void art::RootBranchInfoList::reset(TTree *tree) {
  if (!tree) {
    throw Exception(errors::NullPointerError)
      << "RootInfoBranchList given null TTree pointer.\n";
  }
  TObjArray *branches = tree->GetListOfBranches();
  size_t nBranches = branches->GetEntriesFast();
  data_.clear();
  data_.reserve(nBranches);
  TIter it(branches, kIterBackward);
  // Load the list backward, then searches can take place in the forward
  // direction.
  while (TBranch *b = dynamic_cast<TBranch *>(it.Next())) {
    data_.push_back(RootBranchInfo(b));
  }
  if (nBranches != data_.size()) {
    throw Exception(errors::DataCorruption, "RootBranchInfoList")
      << "Could not read expected number of branches from TTree's list.\n";
  }
}

bool
art::RootBranchInfoList::
findBranchInfo(TypeID const &type,
               InputTag const &tag,
               RootBranchInfo &rbInfo) const {
  std::ostringstream pat_s;
  pat_s << '^'
        << type.friendlyClassName()
        << '_'
        << tag.label()
        << '_'
        << tag.instance()
        << '_';
  if (tag.process().empty()) {
    pat_s << ".*";
  } else {
    pat_s << tag.process();
  }
  pat_s << "\\.$";
  std::regex r(pat_s.str());
  // data_ is ordered so that the first match is the best.
  for (Data_t::const_iterator
         i = data_.begin(),
         e = data_.end();
       i !=e;
       ++i) {
    if (std::regex_match(i->branchName(), r)) {
      rbInfo = *i;
      return true;
    }
  }
  return false;
}
