#include <iostream>
#include "test/Utilities/openmpTestFunc.h"

#include "art/Utilities/hard_cast.h"

extern "C" {
#include <dlfcn.h>
}

int main(int argc, char ** argv)
{
  if (argc < 3) {
    return 1;
  }
  dlerror();
  void * lib_ptr = dlopen(argv[1], RTLD_LAZY | RTLD_GLOBAL);
  if (lib_ptr == nullptr) {
    std::cerr << "Could not load library: " << dlerror() << "\n";
    return 1;
  }
  void * func_ptr = dlsym(lib_ptr, argv[2]);
  char const * error = dlerror();
  if (error != nullptr || func_ptr == nullptr) {
    std::cerr << "Unable to load requested symbol: " << error << "\n";
    return 1;
  }
  typedef size_t(*testFunc_t)(size_t, size_t);
  testFunc_t tf = art::hard_cast<testFunc_t>(func_ptr);
  size_t total = tf(10, 20);
  std::cout << "Total: " << total << std::endl;
}
