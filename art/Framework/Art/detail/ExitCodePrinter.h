#ifndef art_Framework_Art_detail_ExitCodePrinter_h
#define art_Framework_Art_detail_ExitCodePrinter_h

#include "art/Framework/Art/detail/info_success.h"
#include <iostream>

namespace art::detail {
  class ExitCodePrinter {
  public:
    ExitCodePrinter&
    operator=(int exitcode) noexcept
    {
      code_ = exitcode;
      return *this;
    }

    ~ExitCodePrinter() noexcept
    {
      if (code_ != info_success()) {
        std::cout << "Art has completed and will exit with status " << code_
                  << "." << std::endl;
      }
    }

    int
    exitcode() const noexcept
    {
      return code_ == info_success() ? 0 : code_;
    }

  private:
    int code_{};
  };
}

#endif /* art_Framework_Art_detail_ExitCodePrinter_h */

// Local Variables:
// mode: c++
// End:
