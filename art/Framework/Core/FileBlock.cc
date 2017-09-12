#include "art/Framework/Core/FileBlock.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/ResultsPrincipal.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"

#include <memory>
#include <string>

class TTree;

using namespace std;

namespace art {

FileBlock::
FileBlock(FileFormatVersion const& version,
          std::string const& fileName) :
  fileFormatVersion_{version},
  fileName_{fileName}
{}

FileBlock::
FileBlock(FileFormatVersion const& version,
          std::string const& fileName,
          std::unique_ptr<ResultsPrincipal>&& resp,
          cet::exempt_ptr<TTree const> ev,
          bool const fastCopy) :
  fileFormatVersion_{version},
  fileName_{fileName},
  resp_{std::move(resp)},
  tree_{ev},
  fastCopyable_{fastCopy}
{}

FileFormatVersion const&
FileBlock::
fileFormatVersion() const
{
  return fileFormatVersion_;
}

cet::exempt_ptr<TTree const>
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

ResultsPrincipal*
FileBlock::
resultsPrincipal() const
{
  return resp_.get();
}

} // namespace art
