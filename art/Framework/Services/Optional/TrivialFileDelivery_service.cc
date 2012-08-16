#include "art/Framework/Services/Optional/TrivialFileDelivery.h"
#include "art/Framework/Services/Interfaces/FileDeliveryStatus.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <fstream>
using namespace art;
using namespace std;
using fhicl::ParameterSet;

art::TrivialFileDelivery::TrivialFileDelivery
(ParameterSet const & pset, ActivityRegistry &)
  : fileList(extractFileListFromPset(pset))
  , nextFile(fileList.begin())
  , endOfFiles(fileList.end())
{
}

int  art::TrivialFileDelivery::doGetNextFileURI(std::string & uri, double & waitTime)
{
  FileDeliveryStatus stat;
  if (nextFile == endOfFiles) {
    stat = FileDeliveryStatus::NO_MORE_FILES;
    return stat;
  }
  {
    ifstream f(nextFile->c_str());
    if (!f) {
      stat = FileDeliveryStatus::NOT_FOUND;
      ++nextFile;
      return stat;
    }
  }
  uri = prependFileDesignation(*nextFile);
  waitTime = 0.0;
  stat = FileDeliveryStatus::SUCCESS;
  ++nextFile;
  return stat;
}

// The remaining doXXX methods are trivial in this class, ignoring the XXX events.
// The real SAMProtocol concrete class might have real work in these.
void art::TrivialFileDelivery::doUpdateStatus(std::string const &, FileDisposition) {}
void art::TrivialFileDelivery::doOutputFileOpened(std::string const &) {}
void art::TrivialFileDelivery::doOutputModuleInitiated
(std::string const &, ParameterSet const &) {}
void art::TrivialFileDelivery::doOutputFileClosed
(std::string const &, std::string const &) {}
void art::TrivialFileDelivery::doEventSelected
(std::string const &,
 EventID const &,
 HLTGlobalStatus const &) {}

// helper functions
std::vector<std::string>
art::TrivialFileDelivery::extractFileListFromPset(ParameterSet const & pset)
{
  ParameterSet p = pset.get<ParameterSet>("source");
  return p.get< std::vector<std::string> >("fileNames");
  // TODO -- How do we properly throw if either source or fileNames is absent?
  // get() does throw, but is it the right throw and should we be catching it?
}

std::string
art::TrivialFileDelivery::prependFileDesignation(std::string const & name) const
{
  std::string s("file://");
  return s + name;
}

DEFINE_ART_SERVICE(art::TrivialFileDelivery)
