#include "canvas/Utilities/TypeID.h"

#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/FriendlyName.h"
#include "canvas/Utilities/uniform_type_name.h"
#include "tbb/concurrent_unordered_map.h"
#include "TClass.h"
#include <cstddef>
#include <ostream>
#include <string>

using namespace std;

namespace art {

void
TypeID::
print(ostream& os) const
{
  os << className();
}

string
TypeID::
className() const
{
  static tbb::concurrent_unordered_map<size_t, string> s_nameMap;
  auto hash_code = typeInfo().hash_code();
  auto entry = s_nameMap.find(hash_code);
  if (entry == s_nameMap.end()) {
    entry = s_nameMap.emplace(hash_code, uniform_type_name(typeInfo())).first;
  }
  return entry->second;
}

string
TypeID::
friendlyClassName() const
{
  return friendlyname::friendlyName(className());
}

bool
TypeID::
hasDictionary() const
{
  return TClass::HasDictionarySelection(className().c_str());
}

ostream&
operator<<(ostream& os, const TypeID& tid)
{
  tid.print(os);
  return os;
}

} // namespace art

