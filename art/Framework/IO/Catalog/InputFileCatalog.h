#ifndef art_Framework_IO_Catalog_InputFileCatalog_h
#define art_Framework_IO_Catalog_InputFileCatalog_h

// ======================================================================
//
// Class InputFileCatalog. Services to manage InputFile catalog
//
// ======================================================================

#include "art/Framework/IO/Catalog/FileCatalog.h"
#include "art/Framework/Services/FileServiceInterfaces/CatalogInterface.h"
#include "art/Framework/Services/FileServiceInterfaces/FileDeliveryStatus.h"
#include "art/Framework/Services/FileServiceInterfaces/FileTransfer.h"
#include "art/Framework/Services/FileServiceInterfaces/FileTransferStatus.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "fhiclcpp/types/Sequence.h"
#include "fhiclcpp/types/TableFragment.h"

#include <string>
#include <vector>

// ----------------------------------------------------------------------

namespace art {

  enum class FileCatalogStatus {
    SUCCESS,
    NO_MORE_FILES,
    DELIVERY_ERROR,
    TRANSFER_ERROR
  };

  class InputFileCatalog : public FileCatalog {
  public:

    struct Config {
      fhicl::Sequence<std::string> namesParameter { fhicl::Name("fileNames") };
    };

    explicit InputFileCatalog(fhicl::TableFragment<Config> const& config);
    virtual ~InputFileCatalog() = default;
    std::vector<FileCatalogItem> const& fileCatalogItems() const {return fileCatalogItems_;}
    FileCatalogItem const& currentFile() const;
    size_t currentIndex() const;
    bool   getNextFile(int attempts=5);
    bool   hasNextFile(int attempts=5);
    void   rewind();
    void   rewindTo(size_t index);
    bool   isSearchable()       {return searchable_;}
    bool   empty() const        {return fileCatalogItems_.empty();}

    std::vector<std::string> const& fileSources() const { return fileSources_; }

    static constexpr size_t indexEnd {std::numeric_limits<size_t>::max()};

  private:
    bool retrieveNextFile(FileCatalogItem & item, int attempts, bool transferOnly = false);
    FileCatalogStatus retrieveNextFileFromCacheOrService(FileCatalogItem & item);
    FileCatalogStatus transferNextFile(FileCatalogItem & item);

    std::vector<std::string> fileSources_;
    std::vector<FileCatalogItem> fileCatalogItems_ {1};
    FileCatalogItem nextItem_ {};
    size_t fileIdx_ {indexEnd};
    size_t maxIdx_ {};
    bool searchable_ {false}; // update value after the service gets configured
    bool nextFileProbed_ {false};
    bool hasNextFile_ {false};

    ServiceHandle<CatalogInterface> ci_;
    ServiceHandle<FileTransfer> ft_;
  };  // InputFileCatalog

}  // art

// ======================================================================

#endif /* art_Framework_IO_Catalog_InputFileCatalog_h */

// Local Variables:
// mode: c++
// End:
