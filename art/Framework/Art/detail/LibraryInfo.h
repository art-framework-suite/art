#ifndef art_Framework_Art_detail_LibraryInfo_h
#define art_Framework_Art_detail_LibraryInfo_h

#include <string>
#include <utility>

namespace art {
  namespace detail {

    class LibraryInfo {
    private:
      using spec_pair_t = std::pair<std::string, std::string>;
    public:

      LibraryInfo(std::string const& so  , spec_pair_t const& specs,
                  std::string const& path, std::string const& desc,
                  std::string const& prov, std::string const& pt )
        : soName_{so}
        , specs_{specs}
        , path_{path}
        , description_{desc}
        , provider_{prov}
        , pluginType_{pt}
      {}

      bool operator<(LibraryInfo const& li) const
      {
        return this->short_spec() < li.short_spec();
      }

      std::string const& so_name()     const { return soName_; }
      std::string const& short_spec()  const { return specs_.first; }
      std::string const& long_spec()   const { return specs_.second; }
      std::string const& path()        const { return path_; }
      std::string const& description() const { return description_; }
      std::string const& provider()    const { return provider_; }
      std::string const& plugin_type() const { return pluginType_; }

    private:

      std::string soName_;
      spec_pair_t specs_;
      std::string path_;
      std::string description_;
      std::string provider_;
      std::string pluginType_;

    };

    class LibraryInfoMatch {
      std::string val_;
    public:

      LibraryInfoMatch(std::string const& name) : val_{name} {}

      bool operator()(LibraryInfo const& li) const
      {
        return val_ == li.short_spec();
      }

    };

  }
}

#endif

// Local variables:
// mode: c++
// End:
