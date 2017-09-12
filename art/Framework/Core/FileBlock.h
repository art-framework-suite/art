#ifndef art_Framework_Core_FileBlock_h
#define art_Framework_Core_FileBlock_h
// vim: set sw=2 expandtab :

// =======================================================================
// FileBlock: Properties of an input file.
// =======================================================================

#include "art/Framework/Principal/ResultsPrincipal.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"
#include "cetlib/exempt_ptr.h"

#include <memory>
#include <string>

class TTree;

namespace art {

class BranchDescription;
class ResultsPrincipal;

class FileBlock {

public:

  FileBlock() = default;

  FileBlock(FileFormatVersion const&,
            std::string const& fileName);

  FileBlock(FileFormatVersion const&,
            std::string const& fileName,
            std::unique_ptr<ResultsPrincipal>&&,
            cet::exempt_ptr<TTree const> eventTree,
            bool fastCopy);

  FileFormatVersion const&
  fileFormatVersion() const;

  cet::exempt_ptr<TTree const> tree() const;

  bool fastClonable() const;

  std::string const&
  fileName() const;

  void
  setNotFastCopyable();

private:

  // Friends only.
  friend class OutputModule;
  ResultsPrincipal* resultsPrincipal() const;

  FileFormatVersion fileFormatVersion_{};
  std::string fileName_{};
  std::unique_ptr<ResultsPrincipal> resp_{nullptr};
  cet::exempt_ptr<TTree const> tree_{nullptr}; // ROOT owns the tree
  bool fastCopyable_{false};

};

} // namespace art

#endif /* art_Framework_Core_FileBlock_h */

// Local Variables:
// mode: c++
// End:
