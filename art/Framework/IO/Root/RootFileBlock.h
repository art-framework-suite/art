#ifndef art_Framework_IO_Root_RootFileBlock_h
#define art_Framework_IO_Root_RootFileBlock_h

// =======================================================================
// RootFileBlock: Properties of a ROOT input file.
// =======================================================================

#include "art/Framework/Core/FileBlock.h"
#include "cetlib/exempt_ptr.h"

#include <memory>
#include <string>

class TTree;

namespace art {

  class RootFileBlock : public FileBlock {
  public:

    RootFileBlock() = default;

    RootFileBlock(FileFormatVersion const& version,
                  std::string const& fileName,
                  std::unique_ptr<ResultsPrincipal>&& resp,
                  cet::exempt_ptr<TTree const> ev,
                  bool const fastCopy) :
      FileBlock{version, fileName, std::move(resp)},
      tree_{ev},
      fastCopyable_{fastCopy}
    {}

    cet::exempt_ptr<TTree const> tree() const {return tree_;}
    bool fastClonable() const {return fastCopyable_;}

  private:
    cet::exempt_ptr<TTree const> tree_{nullptr}; // ROOT owns the tree
    bool fastCopyable_{false};
  };
}

#endif /* art_Framework_IO_Root_RootFileBlock_h */

// Local Variables:
// mode: c++
// End:
