#ifndef art_Framework_Principal_detail_type_label_for_h
#define art_Framework_Principal_detail_type_label_for_h

#include "art/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Utilities/TypeID.h"

namespace art::detail {
  inline TypeLabel
  type_label_for(TypeID const typeID,
                 std::string const& instance,
                 bool const supportsView,
                 ModuleDescription const& md)
  {
    if (md.isEmulatedModule()) {
      return TypeLabel{typeID, instance, supportsView, md.moduleLabel()};
    }
    return TypeLabel{typeID, instance, supportsView, false};
  };
}

#endif
