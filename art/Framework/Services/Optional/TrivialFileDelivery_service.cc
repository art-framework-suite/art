#include "art/Framework/Services/Optional/TrivialFileDelivery.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Services/FileServiceInterfaces/FileDeliveryStatus.h"
#include "canvas/Utilities/Exception.h"
#include "hep_concurrency/RecursiveMutex.h"

#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

using namespace std;
using namespace art;
using namespace hep::concurrency;

using fhicl::ParameterSet;

namespace art {

  namespace {

    void
    throwIfFileNotExist(char const* fname)
    {
      ifstream f(fname);
      if (!f) {
        throw Exception(errors::CatalogServiceError)
          << "Input file not found: " << fname << ".\n";
      }
    }

  } // unnamed namespace

  TrivialFileDelivery::TrivialFileDelivery(
    TrivialFileDelivery::Parameters const&)
  {}

  void
  TrivialFileDelivery::doConfigure(vector<string> const& items)
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    fileList_ = items;
    nextFile_ = fileList_.begin();
    endOfFiles_ = fileList_.end();
  }

  int
  TrivialFileDelivery::doGetNextFileURI(string& uri, double& waitTime)
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    FileDeliveryStatus stat;
    if (nextFile_ == endOfFiles_) {
      stat = FileDeliveryStatus::NO_MORE_FILES;
      return stat;
    }
    // Look for protocol.
    auto pos = nextFile_->find("://");
    if (pos == string::npos) {
      // Bare filename.
      throwIfFileNotExist(nextFile_->c_str());
      uri = prependFileDesignation(*nextFile_);
    } else if (nextFile_->substr(0, pos) == "file") {
      // file://
      throwIfFileNotExist(nextFile_->c_str() + pos + 3);
      uri = *nextFile_;
    } else {
      // Unknown URI.
      uri = *nextFile_;
    }
    waitTime = 0.0;
    stat = FileDeliveryStatus::SUCCESS;
    ++nextFile_;
    return stat;
  }

  void
  TrivialFileDelivery::doUpdateStatus(string const&, FileDisposition)
  {}

  void
  TrivialFileDelivery::doOutputFileOpened(string const&)
  {}

  void
  TrivialFileDelivery::doOutputModuleInitiated(string const&,
                                               ParameterSet const&)
  {}

  void
  TrivialFileDelivery::doOutputFileClosed(string const&, string const&)
  {}

  void
  TrivialFileDelivery::doEventSelected(string const&,
                                       EventID const&,
                                       HLTGlobalStatus const&)
  {}

  bool
  TrivialFileDelivery::doIsSearchable()
  {
    return true;
  }

  void
  TrivialFileDelivery::doRewind()
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    nextFile_ = fileList_.begin();
  }

  vector<string>
  TrivialFileDelivery::extractFileListFromPset(ParameterSet const& pset)
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    // TODO -- How do we properly throw if either source or fileNames is absent?
    // get() does throw, but is it the right throw and should we be catching it?
    auto const& p = pset.get<ParameterSet>("source");
    auto ret = p.get<vector<string>>("fileNames");
    return ret;
  }

  string
  TrivialFileDelivery::prependFileDesignation(string const& name) const
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    string ret{"file://"};
    ret += name;
    return ret;
  }

} // namespace art

DEFINE_ART_SERVICE_INTERFACE_IMPL(art::TrivialFileDelivery,
                                  art::CatalogInterface)
