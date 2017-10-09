// change
#include "art/Framework/IO/Root/GetFileFormatEra.h"

////////////////////////////////////////////////////////////////////////
// Era Histoy
////////////////////////////////////
//
// In general:
//
// * A change to the persistent format which is both backward- and
// forawrd-compatible requires no change to the version or era.
//
// * A backward-compatible change should change the version.
//
// * A non-backward-compatible change should change the era,
//
// Era            Comments
//
//     -          As forked from CMS.
// ART_2011_a     Initial ART-only format.
//
////////////////////////////////////////////////////////////////////////
namespace art {
  std::string const&
  getFileFormatEra()
  {
    static std::string const era = "ART_2011a";
    return era;
  }
}
