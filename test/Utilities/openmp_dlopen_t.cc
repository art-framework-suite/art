#include <iostream>
#include "test/Utilities/openmpTestFunc.h"

#include <cstring>
extern "C" {
#include <dlfcn.h>
}

int main(int, char **) {
   dlerror();
   //    void *lib_ptr = dlopen("libtest_Utilities.so", RTLD_LAZY | RTLD_GLOBAL);
   void *lib_ptr = dlopen("libtest_Integration_ToyRawProductAnalyzer_module.so", RTLD_LAZY | RTLD_GLOBAL);
   if (lib_ptr == nullptr) {
      std::cerr << "Could not load library: " << dlerror() << "\n";
      return 1;
   }
   void *func_ptr = dlsym(lib_ptr, "openmpTestFunc");
   char const *error = dlerror();
   if (error != nullptr || func_ptr == nullptr) {
      std::cerr << "Unable to load requested symbol: " << error << "\n";
      return 1;
   }
   typedef size_t(*testFunc_t)(size_t, size_t);
   testFunc_t tf = nullptr;
   memcpy(&tf, &func_ptr, sizeof(testFunc_t));
   size_t total = tf(10, 20);
   std::cout << "Total: " << total << std::endl;
}
