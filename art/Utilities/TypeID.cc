/*----------------------------------------------------------------------

----------------------------------------------------------------------*/
#include "art/Utilities/Exception.h"
#include "art/Utilities/FriendlyName.h"
#include "art/Utilities/TypeID.h"
#include "cetlib/demangle.h"

#include "tbb/concurrent_unordered_map.h"

#include "TClass.h"

#include <ostream>

void
art::TypeID::print(std::ostream& os) const {
  os << className();
}

std::string
art::TypeID::className() const {
  static tbb::concurrent_unordered_map<std::size_t, std::string> s_nameMap;
  auto hash_code = typeInfo().hash_code();
  auto entry = s_nameMap.find(hash_code);
  if(s_nameMap.end() == entry) {
    entry = s_nameMap.emplace(hash_code, cet::demangle_symbol(typeInfo().name())).first;
  }
  return entry->second;
}

std::string
art::TypeID::friendlyClassName() const {
  return friendlyname::friendlyName(className());
}

bool
art::TypeID::hasDictionary() const {
  return TClass::HasDictionarySelection(className().c_str());
}

std::ostream&
art::operator<<(std::ostream& os, const TypeID& id) {
  id.print(os);
  return os;
}
