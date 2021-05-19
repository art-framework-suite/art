// vim: set sw=2 expandtab :

#include "art/Framework/Services/FileServiceInterfaces/CatalogInterface.h"
#include "art/Framework/Services/FileServiceInterfaces/FileDeliveryStatus.h"
#include "art/Framework/Services/Registry/ServiceDeclarationMacros.h"
#include "art/Framework/Services/Registry/ServiceDefinitionMacros.h"
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "canvas/Persistency/Common/fwd.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/fwd.h"

#include <fstream>
#include <mutex>
#include <string>
#include <vector>

using namespace std;
using fhicl::ParameterSet;

namespace art {

  namespace {

    void
    throwIfFileNotExist(char const* fname) noexcept(false)
    {
      ifstream f(fname);
      if (!f) {
        throw Exception(errors::CatalogServiceError)
          << "Input file not found: " << fname << ".\n";
      }
    }

  } // unnamed namespace

  class TrivialFileDelivery : public CatalogInterface {
  public:
    struct Config {};
    using Parameters = ServiceTable<Config>;

    TrivialFileDelivery(Parameters const& config);

    void doConfigure(std::vector<std::string> const& items) override;
    int doGetNextFileURI(std::string& uri, double& waitTime) override;
    void doUpdateStatus(std::string const& uri,
                        FileDisposition status) override;
    void doOutputFileOpened(std::string const& module_label) override;
    void doOutputModuleInitiated(std::string const& module_label,
                                 fhicl::ParameterSet const& pset) override;
    void doOutputFileClosed(std::string const& module_label,
                            std::string const& file) override;
    void doEventSelected(std::string const& module_label,
                         EventID const& event_id,
                         HLTGlobalStatus const& acceptance_info) override;
    bool doIsSearchable() override;
    void doRewind() override;

  private:
    std::string prependFileDesignation(std::string const& name) const;

    // Protects all data members.
    mutable std::recursive_mutex mutex_{};
    std::vector<std::string> fileList_{};
    std::vector<std::string>::const_iterator nextFile_{fileList_.cbegin()};
    std::vector<std::string>::const_iterator endOfFiles_{fileList_.cend()};
  };

  TrivialFileDelivery::TrivialFileDelivery(
    TrivialFileDelivery::Parameters const&)
  {}

  void
  TrivialFileDelivery::doConfigure(vector<string> const& items)
  {
    std::lock_guard sentry{mutex_};
    fileList_ = items;
    nextFile_ = fileList_.begin();
    endOfFiles_ = fileList_.end();
  }

  int
  TrivialFileDelivery::doGetNextFileURI(string& uri, double& waitTime)
  {
    std::lock_guard sentry{mutex_};
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
    std::lock_guard sentry{mutex_};
    nextFile_ = fileList_.begin();
  }

  string
  TrivialFileDelivery::prependFileDesignation(string const& name) const
  {
    std::lock_guard sentry{mutex_};
    string ret{"file://"};
    ret += name;
    return ret;
  }

} // namespace art

DECLARE_ART_SERVICE_INTERFACE_IMPL(art::TrivialFileDelivery,
                                   art::CatalogInterface,
                                   SHARED)

DEFINE_ART_SERVICE_INTERFACE_IMPL(art::TrivialFileDelivery,
                                  art::CatalogInterface)
