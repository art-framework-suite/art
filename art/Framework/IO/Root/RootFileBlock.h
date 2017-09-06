#ifndef art_Framework_IO_Root_RootFileBlock_h
#define art_Framework_IO_Root_RootFileBlock_h

#include "art/Framework/Core/FileBlock.h"
#include "cetlib/exempt_ptr.h"

#include <memory>
#include <map>
#include <string>
#include <vector>

class TTree;

namespace art {

  class RootFileBlock : public FileBlock{
  public:

    RootFileBlock() = default;

    RootFileBlock(FileFormatVersion const& version,
                  std::string const& fileName) :
      fileFormatVersion_{version},
      fileName_{fileName}
    {}

    RootFileBlock(FileFormatVersion const& version,
                  cet::exempt_ptr<TTree const> ev,
                  bool fastCopy,
                  std::string const& fileName,
                  std::shared_ptr<BranchChildren> branchChildren,
                  std::unique_ptr<ResultsPrincipal>&& resp = {}) :
      FileBlock{version, fileName},
      tree_{ev},
      fastCopyable_{fastCopy},
      branchChildren_{branchChildren},
      resp_{std::move(resp)}
    {}

    cet::exempt_ptr<TTree const> tree() const {return tree_;}
    bool fastClonable() const {return fastCopyable_;}
    void setNotFastCopyable() {fastCopyable_ = false;}

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

#endif /* art_Framework_Core_RootFileBlock_h */

// Local Variables:
// mode: c++
// End:
