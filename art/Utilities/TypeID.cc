/*----------------------------------------------------------------------

----------------------------------------------------------------------*/
#include "art/Utilities/Exception.h"
#include "art/Utilities/FriendlyName.h"
#include "art/Utilities/TypeID.h"
#include "cetlib/demangle.h"

#include "TClass.h"

#include <ostream>
#include <unordered_map>

namespace {
  std::string typeToClassName(const std::type_info& iType) {
    return cet::demangle_symbol(iType.name());
  }
}

void
art::TypeID::print(std::ostream& os) const {
  os << className();
}

std::string
art::TypeID::className() const {
  thread_local std::unordered_map<std::size_t, std::string> s_nameMap;
  auto hash_code = typeInfo().hash_code();
  auto entry = s_nameMap.find(hash_code);
  if(s_nameMap.end() == entry) {
    entry = s_nameMap.emplace(hash_code, typeToClassName(typeInfo())).first;
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

