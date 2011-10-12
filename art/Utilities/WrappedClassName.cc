#include "art/Utilities/WrappedClassName.h"

namespace art {
  std::string
  wrappedClassName(std::string const & className)
  {
    static std::string const wrapperBegin("art::Wrapper<");
    static std::string const wrapperEnd1(">");
    static std::string const wrapperEnd2(" >");
    std::string const & wrapperEnd = (className[className.size() - 1] == '>' ? wrapperEnd2 : wrapperEnd1);
    std::string wrapped;
    wrapped.reserve(wrapperBegin.size() + className.size() + wrapperEnd.size());
    wrapped += wrapperBegin;
    wrapped += className;
    wrapped += wrapperEnd;
    return wrapped;
  }
}
