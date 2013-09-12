#include "art/Utilities/WrappedClassName.h"

namespace art {
  std::string
  wrappedClassName(std::string const& persistentClassName) {
    static std::string const wrapperBegin("art::Wrapper<");
    static std::string const wrapperEnd1(">");
    static std::string const wrapperEnd2(" >");
    std::string const& wrapperEnd = (persistentClassName[persistentClassName.size()-1] == '>' ? wrapperEnd2 : wrapperEnd1);
    std::string wrapped;
    wrapped.reserve(wrapperBegin.size() + persistentClassName.size() + wrapperEnd.size());
    wrapped += wrapperBegin;
    wrapped += persistentClassName;
    wrapped += wrapperEnd;
    return wrapped;
  }
}
