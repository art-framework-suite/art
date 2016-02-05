#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include <ostream>

void
art::EventAuxiliary::write(std::ostream& os) const {
   os << id_ << std::endl;
}
