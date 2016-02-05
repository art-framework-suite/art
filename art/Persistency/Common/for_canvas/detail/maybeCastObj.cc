#include "canvas/Persistency/Common/detail/maybeCastObj.h"

#include "canvas/Utilities/Exception.h"
#include "cetlib/demangle.h"

#include "TClass.h"
#include "TClassRef.h"

void const *
art::detail::maybeCastObj(void const * address,
                          const std::type_info & tiFrom,
                          const std::type_info & tiTo)
{
  if (tiFrom == tiTo) {
    return address;
  }
  else {
    TClassRef const clFrom(TClass::GetClass(tiFrom));
    TClassRef const clTo(TClass::GetClass(tiTo));

    void const * castAddr(nullptr);

    if (clFrom->InheritsFrom(clTo)) {
      castAddr = clFrom->DynamicCast(clTo, const_cast<void *>(address), true);
    } else if (clTo->InheritsFrom(clFrom)) {
      throw Exception(errors::TypeConversion)
        << "art::Wrapper<> : unable to convert type "
        << cet::demangle_symbol(tiFrom.name())
        << " to "
        << cet::demangle_symbol(tiTo.name())
        << ", which is a subclass.\n";
    }

    if (castAddr != nullptr) {
      return castAddr;
    }
    else {
      throw Exception(errors::TypeConversion)
        << "art::Wrapper<> : unable to convert type "
        << cet::demangle_symbol(tiFrom.name())
        << " to "
        << cet::demangle_symbol(tiTo.name())
        << "\n";
    }
  }
}

