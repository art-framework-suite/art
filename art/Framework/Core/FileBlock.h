#ifndef art_Framework_Core_FileBlock_h
#define art_Framework_Core_FileBlock_h
// vim: set sw=2 expandtab :

// =======================================================================
// FileBlock: Properties of an input file.
// =======================================================================

#include "art/Framework/Principal/ResultsPrincipal.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"

#include <memory>
#include <string>

namespace art {

  class FileBlock {
  public:
    FileBlock();
    virtual ~FileBlock() noexcept;

    FileBlock(FileFormatVersion const& version, std::string const& fileName);
    FileBlock(FileFormatVersion const& version,
              std::string const& fileName,
              std::unique_ptr<ResultsPrincipal>&& resp);

    FileFormatVersion const& fileFormatVersion() const;
    std::string const& fileName() const;

  private:
    friend class OutputModule;
    ResultsPrincipal const* resultsPrincipal() const;

    FileFormatVersion fileFormatVersion_{};
    std::string fileName_{};
    std::unique_ptr<ResultsPrincipal> resp_{};
  };

} // namespace art

#endif /* art_Framework_Core_FileBlock_h */

// Local Variables:
// mode: c++
// End:
