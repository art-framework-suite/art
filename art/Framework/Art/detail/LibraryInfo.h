#ifndef art_Framework_Art_detail_LibraryInfo_h
#define art_Framework_Art_detail_LibraryInfo_h

#include "fhiclcpp/types/ConfigurationTable.h"

#include <memory>
#include <string>
#include <utility>

namespace art::detail {

  class LibraryInfo {
  private:
    using spec_pair_t = std::pair<std::string, std::string>;

  public:
    LibraryInfo(std::string const& so,
                spec_pair_t const& specs,
                std::string const& path,
                std::unique_ptr<fhicl::ConfigurationTable>&& config,
                std::string const& prov,
                std::string const& pt)
      : soName_{so}
      , specs_{specs}
      , path_{path}
      , allowedConfig_{std::move(config)}
      , provider_{prov}
      , pluginType_{pt}
    {}

    bool
    operator<(LibraryInfo const& li) const
    {
      return this->short_spec() < li.short_spec();
    }

    std::string const&
    so_name() const
    {
      return soName_;
    }
    std::string const&
    short_spec() const
    {
      return specs_.first;
    }
    std::string const&
    long_spec() const
    {
      return specs_.second;
    }
    std::string const&
    path() const
    {
      return path_;
    }
    cet::exempt_ptr<fhicl::ConfigurationTable const>
    allowed_config() const
    {
      return allowedConfig_.get();
    }
    std::string const&
    provider() const
    {
      return provider_;
    }
    std::string const&
    plugin_type() const
    {
      return pluginType_;
    }

  private:
    std::string soName_;
    spec_pair_t specs_;
    std::string path_;
    std::unique_ptr<fhicl::ConfigurationTable> allowedConfig_;
    std::string provider_;
    std::string pluginType_;
  };

  class LibraryInfoMatch {
    std::string val_;

  public:
    LibraryInfoMatch(std::string const& name) : val_{name} {}

    bool
    operator()(LibraryInfo const& li) const
    {
      return val_ == li.short_spec();
    }
  };

} // namespace art::detail

#endif /* art_Framework_Art_detail_LibraryInfo_h */

// Local variables:
// mode: c++
// End:
