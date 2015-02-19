#ifndef art_Framework_Core_detail_AvailableLibraries_h
#define art_Framework_Core_detail_AvailableLibraries_h

#include "art/Framework/Core/ModuleType.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <utility>

namespace art {
  namespace detail {

    struct LibraryInfo {

      LibraryInfo(std::string const& name, std::string const& so, std::string const& src, std::string const& type )
        : altname(name),library(so),source(src),type(type) {}

      std::string altname;
      std::string library;
      std::string source;
      std::string type;

    };

    class LibraryInfoContainer {
    public:
      using value_type = std::pair<std::string,LibraryInfo>;

      auto  begin()       { return libs_.begin();  }
      auto  begin() const { return libs_.begin();  }
      auto cbegin() const { return libs_.cbegin(); }

      std::size_t
      count( std::string const& spec ) {
        return std::count_if( libs_.begin(), libs_.end(),
                              [&spec](auto const& entry){ return entry.first == spec; } );
      }

      auto  end()       { return libs_.end();  }
      auto  end() const { return libs_.end();  }
      auto cend() const { return libs_.cend(); }

      bool empty() const { return libs_.empty(); }

      auto
      find_if(std::string const& spec ){
        return std::find_if( libs_.cbegin(), libs_.cend(),
                             [&spec](auto const& entry){return entry.first == spec; } );
      }

      void
      push_back(std::string const& lib, LibraryInfo const& info){
        libs_.push_back( std::make_pair(lib,info) );
      }

      static
      bool
      Comparator( value_type const & lhs, value_type const & rhs ) {
        return lhs.first < rhs.first;
      }

    private:
      std::vector<value_type> libs_;
    };

    constexpr const char* dflt_pattern() {
      return "([-A-Za-z0-9]*_)*[A-Za-z0-9]+_";
    }

    void
    getModuleLibraries( LibraryInfoContainer& map,
                        std::string const & pattern = dflt_pattern());

    void
    getSourceLibraries( LibraryInfoContainer& map,
                        std::string const & pattern = dflt_pattern());

    void
    getServiceLibraries( LibraryInfoContainer& map,
                         std::string const & pattern = dflt_pattern());

  } // detail
} // art

#endif //art_Framework_Core_detail_AvailableLibraries_h

// Local variables:
// mode: c++
// End:
