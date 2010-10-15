#include "art/Utilities/WrappedClassName.h"

namespace art {
  std::string
  wrappedClassName(std::string const& className) {
    std::string const wrapperBegin("art::Wrapper<");
    std::string const wrapperEnd1(">");
    std::string const wrapperEnd2(" >");
    std::string const& wrapperEnd = (className[className.size()-1] == '>' ? wrapperEnd2 : wrapperEnd1);
    std::string wrapped;
    wrapped.reserve(wrapperBegin.size() + className.size() + wrapperEnd.size());
    wrapped += wrapperBegin;
    wrapped += className;
    wrapped += wrapperEnd;
    return wrapped;
  }
}
