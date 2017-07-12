#ifndef art_Utilities_HorizontalRule_h
#define art_Utilities_HorizontalRule_h

#include <string>

namespace art {
  class HorizontalRule {
  public:
    explicit constexpr HorizontalRule(std::size_t const w) : w_{w} {}
    auto operator()(char const fill) const { return std::string(w_, fill); }
  private:
    std::size_t const w_;
  };
}

#endif /* art_Utilities_HorizontalRule_h */

// Local Variables:
// mode: c++
// End:
