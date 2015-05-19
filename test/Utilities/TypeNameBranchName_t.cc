#include "test/Utilities/TypeNameBranchName_t.h"

#include "art/Utilities/Exception.h"
#include "art/Utilities/FriendlyName.h"
#include "cetlib/demangle.h"
#include "cetlib/map_vector.h"

#include "Reflex/Type.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <typeinfo>

namespace {
  std::string reflexName(std::type_info const * const tid) {
    auto t = Reflex::Type::ByTypeInfo(*tid);
    if (!t) {
      throw art::Exception(art::errors::DictionaryNotFound,"NoMatch")
        << "No dictionary for class " << cet::demangle_symbol(tid->name()) << '\n';
    }
    auto cn = t.Name(Reflex::SCOPED);
    auto fn = art::friendlyname::friendlyName(cn);
    return cn + " -> " + fn;
  }
}

int main()
{
  using namespace arttest;
  using namespace std;
  vector<type_info const *> types = { &typeid(string),
                                      &typeid(MyString),
                                      &typeid(vector<string>),
                                      &typeid(vector<MyString>),
                                      &typeid(MyStrings),
                                      &typeid(map<string, int>),
                                      &typeid(map<MyString, int>),
                                      &typeid(MyBigPOD),
                                      &typeid(short),
                                      &typeid(int),
                                      &typeid(uint),
                                      &typeid(long),
                                      &typeid(ulong),
                                      &typeid(cet::map_vector<std::string>),
                                      &typeid(char),
                                      &typeid(unsigned char)
                                    };
  transform(types.cbegin(), types.cend(),
            ostream_iterator<std::string>(std::cout, "\n"),
            &reflexName);
}
