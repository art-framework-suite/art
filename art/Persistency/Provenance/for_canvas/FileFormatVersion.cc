#include "canvas/Persistency/Provenance/FileFormatVersion.h"
#include <ostream>

std::ostream&
operator<< (std::ostream& os, art::FileFormatVersion const& ff) {
   os << (ff.era_.empty()?"":(ff.era_ + ": ")) << ff.value_;
   return os;
}
