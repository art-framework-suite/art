//////////////////////////////////////////////////////////////////////
//
// InputFileCatalog
//
//////////////////////////////////////////////////////////////////////

#include "art/Framework/IO/Catalog/InputFileCatalog.h"
#include "art/Utilities/Exception.h"
#include "boost/algorithm/string.hpp"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"

namespace art {

  InputFileCatalog::InputFileCatalog(fhicl::ParameterSet const& pset,
                                     std::string const& namesParameter,
                                     bool canBeEmpty, bool /*noThrow*/) :
    FileCatalog(),
    logicalFileNames_(canBeEmpty ?
        pset.get<std::vector<std::string> >(namesParameter, std::vector<std::string>()) :
        pset.get<std::vector<std::string> >(namesParameter)),
    fileNames_(logicalFileNames_),
    fileCatalogItems_(),
    fileIter_(fileCatalogItems_.begin()),
    searchable_(true /*update the value after the service gets configured*/) {

    if (logicalFileNames_.empty()) {
      if (canBeEmpty) return;
      throw art::Exception(art::errors::Configuration, "InputFileCatalog::InputFileCatalog()\n")
          << "Empty '" << namesParameter << "' parameter specified for input source.\n";
    }
    // Starting the catalog will write a catalog out if it does not exist.
    // So, do not start (or even read) the catalog unless it is needed.

    fileCatalogItems_.reserve(fileNames_.size());
    typedef std::vector<std::string>::iterator iter;
    for(iter it = fileNames_.begin(), lt = logicalFileNames_.begin(), itEnd = fileNames_.end();
        it != itEnd; ++it, ++lt) {
      boost::trim(*it);
      if (it->empty()) {
        throw art::Exception(art::errors::Configuration, "InputFileCatalog::InputFileCatalog()\n")
          << "An empty string specified in '" << namesParameter << "' parameter for input source.\n";
      }
      if (isPhysical(*it)) {
        lt->clear();
      }
      fileCatalogItems_.push_back(FileCatalogItem(*it, *lt));
    }

    // TODO: configure FileDelivery service
  }

  InputFileCatalog::~InputFileCatalog() {}

  void InputFileCatalog::findFile(std::string & /*pfn*/, std::string const& /*lfn*/, bool /*noThrow*/) {
    cet::exception("You cannot do a logical file lookup! (InputFileCatalog::findFile");
  }

  int  InputFileCatalog::currentPosition() const {
    if( fileIter_==fileCatalogItems_.end() )  return -1;
    return std::distance( fileIter_, fileCatalogItems_.begin() );
  }

  bool InputFileCatalog::getNextFile() {
    // TODO:
    // get next file from FileDelivery service
    // and give it to currentFile_ object
    // returns false if theres no more file
    //
    // If hasNextFile() has been called prior to
    // getNextFile(), it does not actually go fetch
    // the next file from FileDelivery service, 
    // instead, it advances the iterator by one and
    // make the "hidden" next file current.
    return false;
  }

  bool InputFileCatalog::hasNextFile() {
    // TODO:
    // A probe. It tries(and actually does) retreive
    // the next file from the FileDelivery service. But
    // does not advance the current file pointer
    return false;
  }

  void InputFileCatalog::rewind() {
    // TODO: service.rewind()
  }

  void InputFileCatalog::rewindTo(int /*position*/) {
    // TODO: rewind to a previous file location in the catalog
    //       service is not rewinded. only usable when FileDeliveryService::
    //       areFilesPersistent() is true
  }

}  // art
