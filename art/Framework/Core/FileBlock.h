#ifndef art_Framework_Core_FileBlock_h
#define art_Framework_Core_FileBlock_h
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Provenance/FileFormatVersion.h"

#include <memory>
#include <string>

class TTree;

namespace art {

class BranchDescription;
class BranchChildren;
class ResultsPrincipal;

class FileBlock {

public:

  ~FileBlock();

  FileBlock();

  FileBlock(FileFormatVersion const&, std::string const& fileName);

  FileBlock(FileFormatVersion const&, TTree const*, bool fastCopy, std::string const& fileName,
            std::shared_ptr<BranchChildren>, std::unique_ptr<ResultsPrincipal>&& = {});

  FileFormatVersion const&
  fileFormatVersion() const;

  TTree*
  tree() const;

  bool
  fastClonable() const;

  std::string const&
  fileName() const;

  void
  setNotFastCopyable();

  BranchChildren const&
  branchChildren() const;

  ResultsPrincipal*
  resultsPrincipal() const;

private:

  FileFormatVersion
  fileFormatVersion_;

  TTree*
  tree_;

  bool
  fastCopyable_;

  std::string
  fileName_;

  std::shared_ptr<BranchChildren>
  branchChildren_;

  std::unique_ptr<ResultsPrincipal>
  resp_;

};

} // namespace art

#endif /* art_Framework_Core_FileBlock_h */

// Local Variables:
// mode: c++
// End:
