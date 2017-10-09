#include "art/Framework/Services/FileServiceInterfaces/FileDeliveryStatus.h"
#include "art/Framework/Services/Optional/TrivialFileDelivery.h"
#include "canvas/Utilities/Exception.h"
#include <cerrno>
#include <cstdlib>
#include <fstream>
using namespace art;
using namespace std;
using fhicl::ParameterSet;

namespace {
  void
  throwIfFileNotExist(char const* fname)
  {
    std::ifstream f(fname);
    if (!f) {
      // Throw here, otherwise we don't know what file we couldn't find.
      throw Exception(errors::CatalogServiceError)
        << "Input file not found: " << fname << ".\n";
    }
  }
} // namespace

art::TrivialFileDelivery::TrivialFileDelivery(
  TrivialFileDelivery::Parameters const&)
{}

void
art::TrivialFileDelivery::doConfigure(std::vector<std::string> const& items)
{
  fileList = items;
  nextFile = fileList.begin();
  endOfFiles = fileList.end();
}

int
art::TrivialFileDelivery::doGetNextFileURI(std::string& uri, double& waitTime)
{
  FileDeliveryStatus stat;
  if (nextFile == endOfFiles) {
    stat = FileDeliveryStatus::NO_MORE_FILES;
    return stat;
  }
  auto pos = nextFile->find("://"); // Look for protocol.
  if (pos == std::string::npos) {   // Bare filename.
    throwIfFileNotExist(nextFile->c_str());
    uri = prependFileDesignation(*nextFile);
  } else if (nextFile->substr(0, pos) == "file") { // file://
    throwIfFileNotExist(nextFile->c_str() + pos + 3);
    uri = *nextFile;
  } else { // Unknown URI.
    uri = *nextFile;
  }
  waitTime = 0.0;
  stat = FileDeliveryStatus::SUCCESS;
  ++nextFile;
  return stat;
}

// The remaining doXXX methods are trivial in this class, ignoring the XXX
// events. The real SAMProtocol concrete class might have real work in these.
void
art::TrivialFileDelivery::doUpdateStatus(std::string const&, FileDisposition)
{}
void
art::TrivialFileDelivery::doOutputFileOpened(std::string const&)
{}
void
art::TrivialFileDelivery::doOutputModuleInitiated(std::string const&,
                                                  ParameterSet const&)
{}
void
art::TrivialFileDelivery::doOutputFileClosed(std::string const&,
                                             std::string const&)
{}
void
art::TrivialFileDelivery::doEventSelected(std::string const&,
                                          EventID const&,
                                          HLTGlobalStatus const&)
{}

bool
art::TrivialFileDelivery::doIsSearchable()
{
  return true;
}
void
art::TrivialFileDelivery::doRewind()
{
  nextFile = fileList.begin();
}

// helper functions
std::vector<std::string>
art::TrivialFileDelivery::extractFileListFromPset(ParameterSet const& pset)
{
  auto const& p = pset.get<ParameterSet>("source");
  return p.get<std::vector<std::string>>("fileNames");
  // TODO -- How do we properly throw if either source or fileNames is absent?
  // get() does throw, but is it the right throw and should we be catching it?
}

std::string
art::TrivialFileDelivery::prependFileDesignation(std::string const& name) const
{
  std::string const s{"file://"};
  return s + name;
}

DEFINE_ART_SERVICE_INTERFACE_IMPL(art::TrivialFileDelivery,
                                  art::CatalogInterface)
