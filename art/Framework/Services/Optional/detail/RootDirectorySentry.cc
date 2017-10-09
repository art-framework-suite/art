#include "art/Framework/Services/Optional/detail/RootDirectorySentry.h"
#include "TH1.h"

art::detail::RootDirectorySentry::RootDirectorySentry()
  : status_{TH1::AddDirectoryStatus()}
{
  TH1::AddDirectory(true);
}

art::detail::RootDirectorySentry::~RootDirectorySentry() noexcept(false)
{
  TH1::AddDirectory(status_);
}
