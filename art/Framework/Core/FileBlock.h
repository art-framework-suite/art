#ifndef art_Framework_Core_FileBlock_h
#define art_Framework_Core_FileBlock_h

// =======================================================================
// FileBlock: Properties of an input file.
// =======================================================================

#include "art/Framework/Principal/ResultsPrincipal.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"
#include "canvas/Persistency/Provenance/BranchChildren.h"
//#include "cetlib/exempt_ptr.h"

#include <memory>
#include <map>
#include <string>
#include <vector>

class TTree;

namespace art {
  class BranchDescription;
  class FileBlock {
  public:

    FileBlock() = default;

    FileBlock(FileFormatVersion const& version,
              std::string const& fileName) :
      fileFormatVersion_{version},
      fileName_{fileName}
    {}

    FileBlock(FileFormatVersion const& version,
              cet::exempt_ptr<TTree const> ev,
              bool fastCopy,
              std::string const& fileName,
              std::shared_ptr<BranchChildren> branchChildren,
              std::unique_ptr<ResultsPrincipal>&& resp = {}) :
      fileFormatVersion_{version},
      tree_{ev},
      fastCopyable_{fastCopy},
      fileName_{fileName},
      branchChildren_{branchChildren},
      resp_{std::move(resp)}
    {}

    FileFormatVersion const& fileFormatVersion() const {return fileFormatVersion_;}
    cet::exempt_ptr<TTree const> tree() const {return tree_;}

    bool fastClonable() const {return fastCopyable_;}
    std::string const& fileName() const {return fileName_;}

    void setNotFastCopyable() {fastCopyable_ = false;}
    BranchChildren const& branchChildren() const { return *branchChildren_; }

  private:
    // Friends only.
    friend class OutputModule;
    ResultsPrincipal const* resultsPrincipal() const;

    FileFormatVersion fileFormatVersion_{};
    cet::exempt_ptr<TTree const> tree_{nullptr}; // ROOT owns the tree
    bool fastCopyable_{false};
    std::string fileName_{};
    std::shared_ptr<BranchChildren> branchChildren_{std::make_shared<BranchChildren>()};
    std::unique_ptr<ResultsPrincipal> resp_{};
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
