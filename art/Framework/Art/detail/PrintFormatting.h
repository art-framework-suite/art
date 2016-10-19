#ifndef art_Framework_Art_detail_PrintFormatting_h
#define art_Framework_Art_detail_PrintFormatting_h

#include "art/Framework/Art/detail/LibraryInfoCollection.h"
#include "cetlib/container_algorithms.h"

#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace art {
  namespace detail {

    //==========================================================================

    inline std::string indent(std::size_t const i) { return std::string(i,' '); }
    inline std::string indent0()   { return indent(3); }
    inline std::string indent_1()  { return indent(4); }
    inline std::string indent__2() { return indent(8); }

    //==========================================================================

    template <typename T, typename Elem>
    std::size_t columnWidth(T const& coll,
                            std::string const Elem::*cp,
                            std::string const& header)
    {
      std::size_t s {header.size()};
      cet::for_all(coll, [&s,cp](auto const& elem){s = std::max(s,(elem.*cp).size());});
      return s;
    }

    inline std::size_t columnWidth(LibraryInfoCollection const& coll,
                                   std::string const&(LibraryInfo::*fp)() const,
                                   std::string const& header)
    {
      std::size_t s {header.size()};
      cet::for_all(coll, [&s,fp](auto const& li){s = std::max(s,(li.*fp)().size());});
      return s;
    }

    //==========================================================================
    // horizontal rules

    using Widths = std::vector<std::size_t>;

    inline std::size_t rule_size(Widths const& widths)
    {
      std::size_t result {indent0().size()};
      for (std::size_t const w : widths)
        result += w;
      return result += (widths.size()-1)*4u; // Account for space between columns;
    }

    inline auto thick_rule(Widths const& widths)
    {
      return std::string(rule_size(widths),'=');
    }

    inline auto thin_rule(Widths const& widths)
    {
      return std::string(rule_size(widths),'-');
    }

    inline std::string fixed_rule()
    {
      return "\n"+std::string(100,'=')+"\n\n";
    }

  }
}

#endif /* art_Framework_Art_detail_PrintFormatting_h */

// Local variables:
// mode: c++
// End:
