#include "art/Framework/IO/Root/Services/detail/RootDirectorySentry.h"
// vim: set sw=2 expandtab :

#include "TH1.h"

namespace art {
  namespace detail {

    RootDirectorySentry::~RootDirectorySentry() noexcept(false)
    {
      TH1::AddDirectory(status_);
    }

    RootDirectorySentry::RootDirectorySentry()
      : status_{TH1::AddDirectoryStatus()}
    {
      TH1::AddDirectory(true);
    }

  } // namespace detail
} // namespace art
