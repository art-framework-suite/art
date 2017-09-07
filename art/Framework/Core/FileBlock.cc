#include "art/Framework/Core/FileBlock.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/ResultsPrincipal.h"
#include "canvas/Persistency/Provenance/BranchChildren.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"

#include <memory>
#include <string>

class TTree;

using namespace std;

namespace art {

FileBlock::
~FileBlock()
{
  tree_ = nullptr;
}

FileBlock::
FileBlock()
  : fileFormatVersion_{}
  , tree_{nullptr}
  , fastCopyable_{false}
  , fileName_{}
  , branchChildren_{make_shared<BranchChildren>()}
  , resp_{}
{
}

FileBlock::
FileBlock(FileFormatVersion const& version, string const& fileName)
  : fileFormatVersion_{version}
  , tree_{nullptr}
  , fastCopyable_{false}
  , fileName_{fileName}
  , branchChildren_{make_shared<BranchChildren>()}
  , resp_{}
{
}

FileBlock::
FileBlock(FileFormatVersion const& version, TTree const* ev, bool fastCopy, string const& fileName,
          shared_ptr<BranchChildren> branchChildren, unique_ptr<ResultsPrincipal>&& resp /*= {}*/)
  : fileFormatVersion_{version}
  , tree_{const_cast<TTree*>(ev)}
  , fastCopyable_{fastCopy}
  , fileName_{fileName}
  , branchChildren_{branchChildren}
  , resp_{move(resp)}
{
}

FileFormatVersion const&
FileBlock::
fileFormatVersion() const
{
  return fileFormatVersion_;
}

TTree*
FileBlock::
tree() const
{
  return tree_;
}

bool
FileBlock::
fastClonable() const
{
  return fastCopyable_;
}

string const&
FileBlock::
fileName() const
{
  return fileName_;
}

void
FileBlock::
setNotFastCopyable()
{
  fastCopyable_ = false;
}

BranchChildren const&
FileBlock::
branchChildren() const
{
  return *branchChildren_;
}

ResultsPrincipal*
FileBlock::
resultsPrincipal() const
{
  return resp_.get();
}

} // namespace art

