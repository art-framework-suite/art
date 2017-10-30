#include "art/Framework/IO/detail/validateFileNamePattern.h"
#include "canvas/Utilities/Exception.h"

void
art::detail::validateFileNamePattern(bool const do_check,
                                     std::string const& pattern)
{
  if (!do_check)
    return;

  if (pattern.find("%#") == std::string::npos)
    throw Exception(errors::Configuration)
      << "If you have specified the 'fileProperties' table in your configuration,\n"
      << "then the file pattern '%#' MUST be present in the file name.  For "
         "example:\n"
      << "    " << pattern.substr(0, pattern.find(".root")) << "_%#.root\n"
      << "is a supported file name.  Please change your file name to include "
         "the '%#' pattern.";
}
