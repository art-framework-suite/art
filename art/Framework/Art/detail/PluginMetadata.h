#ifndef art_Framework_Art_detail_PluginMetadata_h
#define art_Framework_Art_detail_PluginMetadata_h

#include <string>
#include <vector>

namespace art {
  namespace detail {

    class PluginMetadata {
    public:
      PluginMetadata(std::string const& header,
                     std::string const& details,
                     std::string const& config)
        : header_{header}, details_{details}, config_{config}
      {}

      std::string const&
      header() const
      {
        return header_;
      }
      std::string const&
      details() const
      {
        return details_;
      }
      std::string const&
      allowed_configuration() const
      {
        return config_;
      }

    private:
      std::string header_;
      std::string details_;
      std::string config_;
    };
  }
}

#endif /* art_Framework_Art_detail_PluginMetadata_h */

// Local variables:
// mode: c++
// End:
