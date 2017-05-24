#include "TH1.h"
#include "art/Framework/Services/Optional/detail/RootDirectorySentry.h"


art::detail::RootDirectorySentry::RootDirectorySentry():
  status_{TH1::AddDirectoryStatus()}
{
   TH1::AddDirectory(true);
}

art::detail::RootDirectorySentry::~RootDirectorySentry() noexcept(false)
{
  TH1::AddDirectory(status_);
}
