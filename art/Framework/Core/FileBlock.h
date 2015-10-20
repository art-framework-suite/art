#ifndef art_Framework_Core_FileBlock_h
#define art_Framework_Core_FileBlock_h

/*----------------------------------------------------------------------

FileBlock: Properties of an input file.



----------------------------------------------------------------------*/

#include "art/Framework/Principal/ResultsPrincipal.h"
#include "art/Persistency/Provenance/FileFormatVersion.h"
#include "art/Persistency/Provenance/BranchChildren.h"

class TTree;

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace art {
  class BranchDescription;
  class FileBlock {
  public:
    FileBlock() :
      fileFormatVersion_(),
      tree_(0),
      fastCopyable_(false),
      fileName_(),
      branchChildren_(new BranchChildren)
    {}

    FileBlock(FileFormatVersion const& version,
              std::string const& fileName) :
      fileFormatVersion_(version),
      tree_(0),
      fastCopyable_(false),
      fileName_(fileName),
      branchChildren_(new BranchChildren)
    {}

    FileBlock(FileFormatVersion const& version,
              TTree const* ev,
              bool fastCopy,
              std::string const& fileName,
              std::shared_ptr<BranchChildren> branchChildren,
              std::unique_ptr<ResultsPrincipal> && resp = { }) :
      fileFormatVersion_(version),
      tree_(const_cast<TTree *>(ev)),
      fastCopyable_(fastCopy),
      fileName_(fileName),
      branchChildren_(branchChildren),
      resp_(std::move(resp))
      {}

    // use compiler-generated copy c'tor, copy assignment, and d'tor

    FileFormatVersion const& fileFormatVersion() const {return fileFormatVersion_;}
    TTree * tree() const {return tree_;}

    bool fastClonable() const {return fastCopyable_;}
    std::string const& fileName() const {return fileName_;}

    void setNotFastCopyable() {fastCopyable_ = false;}
    BranchChildren const& branchChildren() const { return *branchChildren_; }

  private:
    // Friends only.
    friend class OutputModule;
    ResultsPrincipal const * resultsPrincipal() const;

    FileFormatVersion fileFormatVersion_;
    // We use bare pointers because ROOT owns these.
    TTree * tree_;
    bool fastCopyable_;
    std::string fileName_;
    std::shared_ptr<BranchChildren> branchChildren_;
    std::unique_ptr<ResultsPrincipal> resp_;
  };
}

inline
art::ResultsPrincipal const *
art::FileBlock::
resultsPrincipal() const
{
  return resp_.get();
}

#endif /* art_Framework_Core_FileBlock_h */

// Local Variables:
// mode: c++
// End:
