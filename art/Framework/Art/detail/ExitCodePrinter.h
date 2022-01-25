#ifndef art_Framework_Art_detail_ExitCodePrinter
#define art_Framework_Art_detail_ExitCodePrinter

#include "art/Framework/Art/detail/info_success.h"
#include <iostream>

namespace art::detail {
  struct ExitCodePrinter {
    int result = 0;
    ~ExitCodePrinter() noexcept
    {
      if (result != info_success()) {
        std::cout << "Art has completed and will exit with status " << result << "." << std::endl;
      }
    }
  };
}

#endif
// Local Variables:
// mode: c++
// End:
