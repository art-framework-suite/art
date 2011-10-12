#include "art/Persistency/Provenance/Hash.h"

namespace art
{
  namespace detail
  {
    // This string is the 16-byte, non-printable version.
    std::string const& InvalidHash()
    {
      static const std::string invalid = art::MD5Result().compactForm();
      return invalid;
    }
  }
}
