#ifndef art_Framework_Art_detail_ServiceNames_h
#define art_Framework_Art_detail_ServiceNames_h

/*
  The existence of this class is unfortunate.  It stems from having to
  tell the user that some configurable system services have .so files
  whose base names differ from those that can be specified in a user's
  configuration file.  For example, the
  'FloatingPointControl_service.so' is loaded in FHiCL by specifying
  'floating_point_control'.  This flexibility in the system should be
  removed so we don't have to go through these unnecessary gymnastics
  and can have uniformity in treatment.

  This class is used ONLY FOR STREAMING to stdout.  Please don't use
  it for anything else.

  KK, June 2015
*/

#include "boost/bimap.hpp"

#include <string>

namespace bimaps = boost::bimaps;

namespace art {
  namespace detail {

    class ServiceNames {
    public:
      using ServiceNames_bimap_t =
        boost::bimap<bimaps::set_of<std::string>, bimaps::set_of<std::string>>;

      static std::string const&
      libname(std::string const& fclname)
      {
        auto it = lookup_.left.find(fclname);
        return it == lookup_.left.end() ? fclname : it->second;
      }

      static std::string const&
      fclname(std::string const& libname)
      {
        auto it = lookup_.right.find(libname);
        return it == lookup_.right.end() ? libname : it->second;
      }

    private:
      static ServiceNames_bimap_t lookup_;
    };

  } // namespace detail
} // namespace art

#endif /* art_Framework_Art_detail_ServiceNames_h */

// Local variables:
// mode: c++
// End:
