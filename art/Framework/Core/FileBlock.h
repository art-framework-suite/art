#ifndef art_Framework_Core_FileBlock_h
#define art_Framework_Core_FileBlock_h

// =======================================================================
// FileBlock: Properties of an input file.
// =======================================================================

#include "art/Framework/Principal/ResultsPrincipal.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"
#include "cetlib/exempt_ptr.h"

#include <memory>
#include <string>

namespace art {

  class FileBlock {
  public:
    FileBlock() = default;
    virtual ~FileBlock() noexcept = default;

    FileBlock(FileFormatVersion const& version, std::string const& fileName)
      : fileFormatVersion_{version}, fileName_{fileName}
    {}

    FileBlock(FileFormatVersion const& version,
              std::string const& fileName,
              std::unique_ptr<ResultsPrincipal>&& resp)
      : fileFormatVersion_{version}, fileName_{fileName}, resp_{std::move(resp)}
    {}

    FileFormatVersion const&
    fileFormatVersion() const
    {
      return fileFormatVersion_;
    }
    std::string const&
    fileName() const
    {
      return fileName_;
    }

  private:
    // Friends only.
    friend class OutputModule;
    ResultsPrincipal const* resultsPrincipal() const;

    FileFormatVersion fileFormatVersion_{};
    std::string fileName_{};
    std::unique_ptr<ResultsPrincipal> resp_{};
  };
}

inline art::ResultsPrincipal const*
art::FileBlock::resultsPrincipal() const
{
  return resp_.get();
}

#endif /* art_Framework_Core_FileBlock_h */

// Local Variables:
// mode: c++
// End:
