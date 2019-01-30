#ifndef art_Utilities_PluginSuffixes_h
#define art_Utilities_PluginSuffixes_h

/*
  'PluginSuffixes' is a static-member only class that contains all
  supported plugins within art (proper).

  ===============
  Developer notes
  ===============

  This class was introduced to support the '--print-description' art
  program option, which queries the list of supported plugin suffixes
  for a given plugin spec as provided by the user.  This way users can
  type:

     art --print-description EmptyEvent TFileService IntProducer RPTest <MyTool>

  where

     EmptyEvent   = source
     TFileService = service
     IntProducer  = module
     RPTest       = plugin

  without having to tell the system what the expected type of each
  specification is.

  Having a container that contains each of these plugin suffixes is
  thus required.  For type-safety reasons, the enum class
  'suffix_type' is used as the map key.

  This class is an all-static member class.  Wherever the
  string-rendered suffix is required within art, it is desirable to do
  something like 'Suffixes::module()'.  This could be achieved,
  clearly, by introducing a "Suffixes" namespace and just using free
  functions.  However, doing so would:

  1. Introduce potential confusion via a using directive, such as
     'using namespace art::Suffixes'.  A call to 'module()' could be
     unclear to the developer.

  2. Require creating a free function that returns the full list of
     allowed suffixes.  Then for each

        'Suffixes::{module,service,source,plugin,tool,mfPlugin,mfStatsPlugin}()'

     free function call, the full-list function would be called each
     time, which seems unnecessary.
*/

#include "canvas/Utilities/Exception.h"

#include <ostream>
#include <string>
#include <type_traits>
#include <vector>

namespace art {

  enum class suffix_type : std::size_t {
    module,
    plugin,
    service,
    source,
    tool,
    mfPlugin,
    mfStatsPlugin
  };

  inline std::ostream&
  operator<<(std::ostream& os, suffix_type const st)
  {
    return os << static_cast<std::underlying_type_t<suffix_type>>(st);
  }

  class Suffixes {
    static constexpr auto
    index_for(suffix_type const st)
    {
      return static_cast<std::underlying_type_t<suffix_type>>(st);
    }

  public:
    static std::string const&
    module()
    {
      return suffixes_[index_for(suffix_type::module)];
    }
    static std::string const&
    plugin()
    {
      return suffixes_[index_for(suffix_type::plugin)];
    }
    static std::string const&
    service()
    {
      return suffixes_[index_for(suffix_type::service)];
    }
    static std::string const&
    source()
    {
      return suffixes_[index_for(suffix_type::source)];
    }
    static std::string const&
    tool()
    {
      return suffixes_[index_for(suffix_type::tool)];
    }
    static std::string const&
    mfPlugin()
    {
      return suffixes_[index_for(suffix_type::mfPlugin)];
    }
    static std::string const&
    mfStatsPlugin()
    {
      return suffixes_[index_for(suffix_type::mfStatsPlugin)];
    }

    static std::string
    print()
    {
      std::string result;
      for (auto const& suffix : suffixes_)
        result += "\n    '" + suffix + "'";
      return result;
    }

    static std::vector<std::string> const&
    all()
    {
      return suffixes_;
    }

  private:
    static std::vector<std::string> const suffixes_;
  };

} // namespace art

#endif /* art_Utilities_PluginSuffixes_h */

// Local variables:
// mode: c++
// End:
