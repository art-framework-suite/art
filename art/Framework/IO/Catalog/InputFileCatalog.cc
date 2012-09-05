//////////////////////////////////////////////////////////////////////
//
// InputFileCatalog
//
//////////////////////////////////////////////////////////////////////

#include "art/Framework/IO/Catalog/InputFileCatalog.h"
#include "art/Framework/Services/Optional/TrivialFileDelivery.h"
#include "art/Framework/Services/Optional/TrivialFileTransfer.h"
#include "art/Utilities/Exception.h"
#include "boost/algorithm/string.hpp"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"

#include <limits>

const size_t art::InputFileCatalog::indexEnd = std::numeric_limits<size_t>::max();

namespace art {

  InputFileCatalog::InputFileCatalog(fhicl::ParameterSet const& pset,
                                     std::string const& namesParameter,
                                     bool canBeEmpty, bool /*noThrow*/) :
    FileCatalog(),
    fileSources_(canBeEmpty ?
        pset.get<std::vector<std::string> >(namesParameter, std::vector<std::string>()) :
        pset.get<std::vector<std::string> >(namesParameter)),
    fileCatalogItems_(1),
    fileIdx_(indexEnd),
    maxIdx_(0),
    searchable_(false /*update the value after the service gets configured*/),
    nextFileProbed_(false),
    hasNextFile_(false) {

    if (fileSources_.empty() && !canBeEmpty) {
      throw art::Exception(art::errors::Configuration, "InputFileCatalog::InputFileCatalog()\n")
          << "Empty '" << namesParameter << "' parameter specified for input source.\n";
    }

    // Configure FileDelivery service
    tfd_->configure(fileSources_);
    searchable_ = tfd_->isSearchable();

    if( searchable_ ) fileCatalogItems_.resize(fileSources_.size());
  }

  InputFileCatalog::~InputFileCatalog() {}

  void InputFileCatalog::findFile(std::string & /*pfn*/, std::string const& /*lfn*/, bool /*noThrow*/) {
    cet::exception("You cannot do a logical file lookup! (InputFileCatalog::findFile");
  }

  FileCatalogItem const & InputFileCatalog::currentFile() const {
    if( fileIdx_==indexEnd ) {
      throw art::Exception(art::errors::LogicError, 
        "Cannot access the current file while the file catalog is empty!");
    }
    assert( fileIdx_ <= maxIdx_ );
    return fileCatalogItems_[fileIdx_];
  }

  size_t InputFileCatalog::currentIndex() const {
    return fileIdx_;
  }

  bool InputFileCatalog::getNextFile() {
    // get next file from FileDelivery service
    // and give it to currentFile_ object
    // returns false if theres no more file
    //
    // If hasNextFile() has been called prior to
    // getNextFile(), it does not actually go fetch
    // the next file from FileDelivery service, 
    // instead, it advances the iterator by one and
    // make the "hidden" next file current.

    if( nextFileProbed_ && !hasNextFile_ )
      return false;

    if( (nextFileProbed_ && hasNextFile_) || retrieveNextFileFromCacheOrService(nextItem_) ) 
    {
      nextFileProbed_ = false;
      fileIdx_ = (fileIdx_ == indexEnd) ? 0 : (searchable_ ? (fileIdx_+1) : 0);
      if( fileIdx_ > maxIdx_ ) maxIdx_ = fileIdx_;
      fileCatalogItems_[fileIdx_] = nextItem_;
      return true;
    }

    return false;
  }

  bool InputFileCatalog::hasNextFile() {
    // A probe. It tries(and actually does) retreive
    // the next file from the FileDelivery service. But
    // does not advance the current file pointer
    if( nextFileProbed_ )
      return hasNextFile_;

    hasNextFile_ = retrieveNextFileFromCacheOrService(nextItem_);
    nextFileProbed_ = true;
    return hasNextFile_;
  }

  bool InputFileCatalog::retrieveNextFileFromCacheOrService(FileCatalogItem & item) {
    // Try to get it from cached files
    if( fileIdx_ < maxIdx_ ) {
      item = fileCatalogItems_[fileIdx_+1];
      return true;
    }
      
    // Try to get it from the service
    std::string uri, pfn;
    double wait = 0.0;

    if( tfd_->getNextFileURI( uri, wait ) != FileDeliveryStatus::SUCCESS )
      return false;

    if( tft_->translateToLocalFilename( uri, pfn ) != FileTransferStatus::CREATED )
      return false;

    std::string lfn = pfn;
   
    boost::trim(pfn);

    if (pfn.empty()) {
      throw art::Exception(art::errors::Configuration, 
        "InputFileCatalog::retrieveNextFileFromCacheService()\n")
        << "An empty string specified in parameter for input source.\n";
    }

    if (isPhysical(pfn)) {
      lfn.clear();
    }

    item = FileCatalogItem(pfn, lfn);
    return true;
  }

  void InputFileCatalog::rewind() {
    if ( !searchable_ ) {
      throw art::Exception(art::errors::LogicError, "InputFileCatalog::rewind()\n")
        << "A non-searchable catalog is not allowed to rewind!";
    }
    fileIdx_ = 0;
  }

  void InputFileCatalog::rewindTo(size_t index) {
    // rewind to a previous file location in the catalog
    // service is not rewinded. only usable when FileDeliveryService::
    // areFilesPersistent() is true
    if ( !searchable_ ) {
      throw art::Exception(art::errors::LogicError, "InputFileCatalog::rewindTo()\n")
        << "A non-searchable catalog is not allowed to rewind!";
    }

    if( index > maxIdx_ ) {
      throw art::Exception(art::errors::InvalidNumber, "InputFileCatalog::rewindTo()\n")
        << "Index " << index << " is out of range!";
    }

    fileIdx_ = index;
  }

}  // art
