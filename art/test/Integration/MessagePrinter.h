#ifndef art_test_Integration_MessagePrinter_h
#define art_test_Integration_MessagePrinter_h

#include <ostream>
#include <string>

namespace fhicl {
  class ParameterSet;
}

namespace arttest {
  class MessagePrinter {
  public:
    MessagePrinter(fhicl::ParameterSet const&) {}
    void
    print(std::ostream& os, std::string const& msg)
    {
      os << msg;
    }
  };
}

#endif /* art_test_Integration_MessagePrinter_h */

// Local variables:
// mode: c++
// End:
