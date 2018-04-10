#include "cetlib/loadable_libraries.h"

#include <iostream>
#include <string>

namespace {
  std::string const message{"message"};
  std::string const scheduler{"scheduler"};
}

int
main(int argc, char** argv)
{
  if (argc < 2 || argc > 3) {
    return 1;
  }
  std::string const suffix{argv[1]};
  std::string const spec{argc == 2 ? "" : argv[2]};
  cet::loadable_libraries(std::cout, spec, suffix);
  if (suffix.find("service") == std::string::npos)
    return 0;

  // Special cases that are not services, per se, but nonetheless are
  // configured in the services configuration table.
  if (message.find(spec) == 0) {
    std::cout << "message\n";
  }
  if (scheduler.find(spec) == 0) {
    std::cout << "scheduler\n";
  }
}
