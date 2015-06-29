#ifndef art_Framework_Core_detail_ServiceNames_h
#define art_Framework_Core_detail_ServiceNames_h

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
      ServiceNames()
      {
        using position = typename decltype(lookup_)::value_type;

        // These services have invocations in FHiCL that are different than
        // the .so base name
        lookup_.insert( position("floating_point_control","FloatingPointControl") );
      }

      inline std::string libname( std::string const & fclname ) const
      {
        auto it = lookup_.left.find( fclname );
        return it == lookup_.left.end() ? fclname : it->second;
      }

      inline std::string fclname( std::string const & libname ) const
      {
        auto it = lookup_.right.find( libname );
        return it == lookup_.right.end() ? libname : it->second;
      }

      // iterators for printed names
      auto begin() const { return lookup_.right.begin(); }
      auto end()   const { return lookup_.right.end(); }

    private:
      boost::bimap<bimaps::set_of<std::string>,bimaps::set_of<std::string> > lookup_;
    };

  }
}

#endif

// Local variables:
// mode: c++
// End:
