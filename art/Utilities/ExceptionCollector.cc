#include "art/Utilities/ExceptionCollector.h"

namespace art {
  void
  ExceptionCollector::rethrow() const {
    throw exception_;
  }

  void
  ExceptionCollector::call(boost::function<void(void)> f) {
    try {
      f();
    }
    catch (artZ::Exception e) {
      hasThrown_ = true;
      exception_ << e;
    }
    catch (std::exception e) {
      hasThrown_ = true;
      exception_ << e.what();
    }
    catch (...) {
      hasThrown_ = true;
      exception_ << "Unknown exception";
    }
  }
}
