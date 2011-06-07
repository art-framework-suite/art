#ifndef art_Framework_Core_FileBlock_h
#define art_Framework_Core_FileBlock_h

/*----------------------------------------------------------------------

FileBlock: Properties of an input file.



----------------------------------------------------------------------*/

#include "art/Persistency/Provenance/FileFormatVersion.h"
#include "art/Persistency/Provenance/BranchChildren.h"
class TTree;
#include "cpp0x/memory"
#include <map>
#include <string>
#include <vector>

namespace art {
  class BranchDescription;
  class FileBlock {
  public:
    FileBlock() :
      fileFormatVersion_(),
      tree_(0),
      metaTree_(0),
      subRunTree_(0),
      subRunMetaTree_(0),
      runTree_(0),
      runMetaTree_(0),
      fastCopyable_(false),
      fileName_(),
      branchChildren_(new BranchChildren)
    {}

    FileBlock(FileFormatVersion const& version,
              std::string const& fileName) :
      fileFormatVersion_(version),
      tree_(0),
      metaTree_(0),
      subRunTree_(0),
      subRunMetaTree_(0),
      runTree_(0),
      runMetaTree_(0),
      fastCopyable_(false),
      fileName_(fileName),
      branchChildren_(new BranchChildren)
    {}

    FileBlock(FileFormatVersion const& version,
	      TTree const* ev, TTree const* meta,
	      TTree const* subRun, TTree const* subRunMeta,
	      TTree const* run, TTree const* runMeta,
	      bool fastCopy,
	      std::string const& fileName,
	      std::shared_ptr<BranchChildren> branchChildren) :
      fileFormatVersion_(version),
      tree_(const_cast<TTree *>(ev)),
      metaTree_(const_cast<TTree *>(meta)),
      subRunTree_(const_cast<TTree *>(subRun)),
      subRunMetaTree_(const_cast<TTree *>(subRunMeta)),
      runTree_(const_cast<TTree *>(run)),
      runMetaTree_(const_cast<TTree *>(runMeta)),
      fastCopyable_(fastCopy),
      fileName_(fileName),
      branchChildren_(branchChildren) {}

    // use compiler-generated copy c'tor, copy assignment, and d'tor

    FileFormatVersion const& fileFormatVersion() const {return fileFormatVersion_;}
    TTree * tree() const {return tree_;}
    TTree * metaTree() const {return metaTree_;}
    TTree * subRunTree() const {return subRunTree_;}
    TTree * subRunMetaTree() const {return subRunMetaTree_;}
    TTree * runTree() const {return runTree_;}
    TTree * runMetaTree() const {return runMetaTree_;}

    bool fastClonable() const {return fastCopyable_;}
    std::string const& fileName() const {return fileName_;}

    void setNotFastCopyable() {fastCopyable_ = false;}
    BranchChildren const& branchChildren() const { return *branchChildren_; }

  private:
    FileFormatVersion fileFormatVersion_;
    // We use bare pointers because ROOT owns these.
    TTree * tree_;
    TTree * metaTree_;
    TTree * subRunTree_;
    TTree * subRunMetaTree_;
    TTree * runTree_;
    TTree * runMetaTree_;
    bool fastCopyable_;
    std::string fileName_;
    std::shared_ptr<BranchChildren> branchChildren_;
  };
}
#endif /* art_Framework_Core_FileBlock_h */

// Local Variables:
// mode: c++
// End:
