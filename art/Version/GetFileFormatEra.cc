// change
#include "art/Version/GetFileFormatEra.h"

namespace art
{
   std::string const &getFileFormatEra() {
      static std::string const era = "ART_2011a";
      return era;
   }
}
