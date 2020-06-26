#ifndef art_Framework_IO_detail_validateFileNamePattern_h
#define art_Framework_IO_detail_validateFileNamePattern_h

#include "fhiclcpp/types/Atom.h"
#include <string>

namespace art::detail {
  struct SafeFileNameConfig {
    fhicl::Atom<bool> checkFileName{
      fhicl::Name("checkFileName"),
      fhicl::Comment(
        "If file-switching has been enabled, the output filename pattern\n"
        "must have a '%#' format specifier by default.  This ensures that\n"
        "an output file will never be overwritten for a given job "
        "execution.\n\n"
        "For some workflows, this restriction is unnecessary if the user "
        "can\n"
        "guarantee that the specified filename pattern will create distinct\n"
        "output files.\n\n"
        "If 'requireFilePatternCheck' is set to 'false', then the '%#' is\n"
        "not required even if file-switching has been enabled.  In this "
        "mode\n"
        "it is the user's responsibility to supply a filename pattern that\n"
        "will not result in output-file overwriting.\n\n"
        "Do NOT set this to 'false' unless you know what you are doing!"),
      true};
  };
  void validateFileNamePattern(bool do_check, std::string const& pattern);
}

#endif /* art_Framework_IO_detail_validateFileNamePattern_h */

// Local Variables:
// mode: c++
// End:
