#include "art/Framework/Services/Optional/TrivialFileTransfer.h"
#include "art/Framework/Services/Interfaces/FileTransferStatus.h"
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iterator>  // for istream_iterator and ostream_iterator
#include <algorithm> // for std::copy
#include <stdio.h>   // for tmpnam()
#include <cstdlib>   // for getenv()
using namespace art;
using namespace std;
using fhicl::ParameterSet;

art::TrivialFileTransfer::TrivialFileTransfer
(ParameterSet const & , ActivityRegistry &)
  : scratchArea("."/*std::getenv("ART_FILE_TRANSFER_SCRATCH_AREA")*/)
{
}

int art::TrivialFileTransfer::doTranslateToLocalFilename
(std::string const & uri, std::string & fileFQname)
{
  FileTransferStatus stat = FileTransferStatus::OK; // Note that in our context, OK does not denote
  // successful file *creation*
  fileFQname = "";
  std::string inFileName;
  int inFileStatus = stripURI(uri, inFileName);
  if (inFileStatus != 0) {
    stat = FileTransferStatus::BADREQUEST;
    return stat;
  }
  ifstream infile(inFileName.c_str());
  if (!infile) {
    stat = FileTransferStatus::NOTFOUND;
    return stat;
  }
#if 0
  std::string ofileName = scratchArea + "/" + tmpnam(NULL);
  ofstream outfile(ofileName.c_str());
  if (!outfile) {
    stat = FileTransferStatus::FORBIDDEN;
    return stat;
  }
  int copystat = copyFile(infile, outfile);
  if (copystat != 0) {
    stat = FileTransferStatus::FORBIDDEN;
    return stat;
  }
  fileFQname = ofileName;
#endif
  fileFQname = inFileName;
  stat = FileTransferStatus::CREATED;
  return stat;
  // Implementation plan details -- alternatives not chosen:
  // x We could merely return the file name (the URI with file:// stripped off).
  //   Since the SAM developers may look at this file as a template for their real
  //   GeneralFileTransfer service, it is perhaps better to do the work of making
  //   a copy into a designated area.
  // x We  merely strip the file:// from the URI; this adhoc class is not beefed
  //   up to deal with genuine web access.
  // x An alternative would be to embed the last part of the file FQname into the
  //   scratch file name, to try to maintain traceability in case things break.
}

int art::TrivialFileTransfer::stripURI
(std::string const & uri, std::string & inFileName) const
{
  const std::string filestr("file://");
  if (uri.substr(0, 7) != filestr) {
    inFileName = "";
    return 1;
  }
  inFileName = uri.substr(7);
  return 0;
}

int art::TrivialFileTransfer::copyFile(std::ifstream & in, std::ofstream & out) const
{
  std::copy(std::istream_iterator<char>(in),
            std::istream_iterator<char>(),
            std::ostream_iterator<char>(out));
  return 0;
}

DEFINE_ART_SERVICE_INTERFACE_IMPL(art::TrivialFileTransfer, art::FileTransfer)
