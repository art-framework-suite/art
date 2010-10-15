#ifndef FWCore_Framework_FileBlock_h
#define FWCore_Framework_FileBlock_h

/*----------------------------------------------------------------------

FileBlock: Properties of an input file.



----------------------------------------------------------------------*/

#include "art/Persistency/Provenance/FileFormatVersion.h"
#include "art/Persistency/Provenance/BranchChildren.h"
class TTree;
#include "boost/shared_ptr.hpp"
#include <map>
#include <string>
#include <vector>

namespace art {
  class BranchDescription;
  class FileBlock {
  public:
    FileBlock() :
      fileFormatVersion_(),
      tree_(0), metaTree_(0),
      subRunTree_(0), subRunMetaTree_(0),
      runTree_(0), runMetaTree_(0),
      fastCopyable_(false), fileName_(),
      branchChildren_(new BranchChildren) {}

    FileBlock(FileFormatVersion const& version,
	      TTree const* ev, TTree const* meta,
	      TTree const* subRun, TTree const* subRunMeta,
	      TTree const* run, TTree const* runMeta,
	      bool fastCopy,
	      std::string const& fileName,
	      boost::shared_ptr<BranchChildren> branchChildren) :
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

    ~FileBlock() {}

    FileFormatVersion const& fileFormatVersion() const {return fileFormatVersion_;}
    TTree * const tree() const {return tree_;}
    TTree * const metaTree() const {return metaTree_;}
    TTree * const subRunTree() const {return subRunTree_;}
    TTree * const subRunMetaTree() const {return subRunMetaTree_;}
    TTree * const runTree() const {return runTree_;}
    TTree * const runMetaTree() const {return runMetaTree_;}

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
    boost::shared_ptr<BranchChildren> branchChildren_;
  };
}
#endif
