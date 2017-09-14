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
          std::unique_ptr<ResultsPrincipal>&& resp) :
  fileFormatVersion_{version},
  fileName_{fileName},
  resp_{std::move(resp)}
{}

FileFormatVersion const&
FileBlock::
fileFormatVersion() const
{
  return fileFormatVersion_;
}

string const&
FileBlock::
fileName() const
{
  return fileName_;
}

ResultsPrincipal const*
FileBlock::
resultsPrincipal() const
{
  return resp_.get();
}

} // namespace art
