#include "art/Framework/IO/Sources/detail/FileServiceProxy.h"

art::detail::FileServiceProxy::FileServiceProxy(
  std::vector<std::string>&& fileNames,
  size_t const attempts,
  double const waitBetweenAttempts)
  : currentItem_{attempts}
  , attemptsPerPhase_{attempts}
  , waitBetweenAttempts_{waitBetweenAttempts}
{
  ci_->configure(std::move(fileNames));
}

art::detail::FileServiceProxy::~FileServiceProxy()
{
  if (!currentItem_.uri.empty()) {
    if (currentItem_.ftStatus == FileTransferStatus::SUCCESS) {
      ci_->updateStatus(currentItem_.uri, FileDisposition::INCOMPLETE);
    } else {
      ci_->updateStatus(currentItem_.uri, FileDisposition::SKIPPED);
    }
  }
}

void
art::detail::FileServiceProxy::finish()
{
  if (currentItem_.ftStatus == FileTransferStatus::SUCCESS) {
    // File is complete.
    ci_->updateStatus(currentItem_.uri, FileDisposition::CONSUMED);
  }
  currentItem_ = FileEntity(attemptsPerPhase_);
}

std::string
art::detail::FileServiceProxy::obtainURI_()
{
  double wait = 0.0;
  switch (currentItem_.uriStatus) {
    case FileDeliveryStatus::TRY_AGAIN_LATER:
      [[fallthrough]];
    case FileDeliveryStatus::UNAVAILABLE:
      wait = waitBetweenAttempts_;
      [[fallthrough]];
    case FileDeliveryStatus::PENDING:
      if (!currentItem_.attemptsRemaining--) {
        throw Exception(errors::CatalogServiceError)
          << "Unable to obtain URI from CatalogInterface service after "
          << attemptsPerPhase_ << " attempts.\n";
      }
      currentItem_.uriStatus = static_cast<FileDeliveryStatus>(
        ci_->getNextFileURI(currentItem_.uri, wait));
      return obtainURI_();
    case FileDeliveryStatus::SUCCESS:
      currentItem_.attemptsRemaining = attemptsPerPhase_; // Reset.
      return obtainFileFromURI_();
      break;
    case FileDeliveryStatus::NO_MORE_FILES:
      return std::string(); // Done.
    default:
      throw Exception(errors::CatalogServiceError)
        << "CatalogInterface service returned failure code "
        << translateFileDeliveryStatus(currentItem_.uriStatus) << " ("
        << static_cast<int>(currentItem_.uriStatus) << ").\n";
  }
}

std::string
art::detail::FileServiceProxy::obtainFileFromURI_()
{
  assert(currentItem_.uriStatus == FileDeliveryStatus::SUCCESS);
  switch (currentItem_.ftStatus) {
    case FileTransferStatus::UNAVAILABLE:
      sleep(waitBetweenAttempts_);
      [[fallthrough]];
    case FileTransferStatus::PENDING:
      if (!currentItem_.attemptsRemaining--) {
        throw Exception(errors::CatalogServiceError)
          << "Unable to obtain URI " << currentItem_.uri
          << " from FileTransfer service after " << attemptsPerPhase_
          << " attempts.\n";
      }
      currentItem_.ftStatus = static_cast<FileTransferStatus>(
        ft_->translateToLocalFilename(currentItem_.uri, currentItem_.pfn));
      return obtainFileFromURI_();
    case FileTransferStatus::SUCCESS:
      ci_->updateStatus(currentItem_.uri, FileDisposition::TRANSFERRED);
      return currentItem_.pfn; // Done.
    default:
      throw Exception(errors::CatalogServiceError)
        << "FileTransfer service returned failure code "
        << translateFileTransferStatus(currentItem_.ftStatus) << " ("
        << static_cast<int>(currentItem_.ftStatus) << ") for URI "
        << currentItem_.uri << ".\n";
  }
}
