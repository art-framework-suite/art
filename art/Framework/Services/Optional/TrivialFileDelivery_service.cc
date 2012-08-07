#include "art/Framework/Services/UserInteraction/AdhocFileDelivery.h"
#include "art/Framework/Services/UserInteraction/FileDeliveryStatus.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <fstream>
using namespace art;
using namespace std;
using fhicl::ParameterSet;

art::AdhocFileDelivery::AdhocFileDelivery 
   ( ParameterSet const & pset, ActivityRegistry & )
     : fileList(extractFileListFromPset(pset))
     , nextFile(fileList.begin())
     , endOfFiles(fileList.end())
{
}

int  art::AdhocFileDelivery::doGetNextFileURI(URI & uri, double & waitTime)
{
  FileDeliveryStatus stat;
  if (nextFile == endOfFiles) {
    stat = NO_MORE_FILES;
    return stat;
  }
  { ifstream f(nextFile->c_str());
    if (!f) {
      stat = NOT_FOUND;
      ++nextFile;
      return stat;
    } 
  }
  uri = prependFileDesignation(*nextFile);
  waitTime = 0.0;
  stat = SUCCESS;
  ++nextFile;
  return stat;
}

// The remaining doXXX methods are trivial in this class, ignoring the XXX events.
// The real SAMProtocol concrete class might have real work in these.
void art::AdhocFileDelivery::doUpdateStatus (URI const & , int ) {}
void art::AdhocFileDelivery::doOutputFileOpened (module_id_t const & ){}
 void art::AdhocFileDelivery::doOutputModuleInitiated 
    (module_id_t const & , ParameterSet const & ) {}  
void art::AdhocFileDelivery::doOutputFileClosed 
    (module_id_t const & , URI const & ) {}
void art::AdhocFileDelivery::doEventSelected
    (module_id_t const & ,
     EventID , HLTGlobalStatus ) {}
  
// helper functions
std::vector<std::string> 
art::AdhocFileDelivery::extractFileListFromPset(ParameterSet const & pset)
{
  ParameterSet p = pset.get<ParameterSet>("source"); 
  return p.get< std::vector<std::string> >("fileNames");
  // TODO -- How do we properly throw if either source or fileNames is absent?
  // get() does throw, but is it the right throw and should we be catching it? 
}

std::string 
  art::AdhocFileDelivery::prependFileDesignation(std::string const & name) const
{
  std::string s ("file://");
  return s+name;
}



DEFINE_ART_SERVICE(art::AdhocFileDelivery)
