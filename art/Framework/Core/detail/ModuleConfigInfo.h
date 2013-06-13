#ifndef art_Framework_Core_detail_ModuleConfigInfo_h
#define art_Framework_Core_detail_ModuleConfigInfo_h

#include "art/Framework/Core/ModuleType.h"
#include "art/Framework/Core/WorkerInPath.h"
#include "fhiclcpp/ParameterSet.h"

#include <map>
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////
// ModuleConfigInfo
//
// Class containing some info about a module only required during
// configuration processing.
////////////////////////////////////////////////////////////////////////

namespace art {
  namespace detail {
    class ModuleConfigInfo;

    typedef std::map<std::string, ModuleConfigInfo> ModuleConfigInfoMap;
  }

}

class art::detail::ModuleConfigInfo {
public:
  ModuleConfigInfo(fhicl::ParameterSet const & procPS,
                   std::string const & labelInPathConfig,
                   std::string const & configPath);

  std::string const & labelInPathConfig() const;
  std::string const & configPath() const;
  WorkerInPath::FilterAction filterAction() const;
  std::string const & simpleLabel() const;
  ModuleType moduleType() const;
  std::string const & libSpec() const;

  static
  std::vector<std::string> const & allModulePathRoots();

private:
  WorkerInPath::FilterAction calcFilterAction_() const;
  std::string calcSimpleLabel_() const;
  ModuleType calcConfigType_() const;

  std::string labelInPathConfig_;
  std::string configPath_;
  WorkerInPath::FilterAction filterAction_;
  std::string simpleLabel_;
  ModuleType moduleType_;
  std::string libSpec_;
};

inline
std::string const &
art::detail::ModuleConfigInfo::
labelInPathConfig() const
{
  return labelInPathConfig_;
}

inline
std::string const &
art::detail::ModuleConfigInfo::
configPath() const
{
  return configPath_;
}

inline
art::WorkerInPath::FilterAction
art::detail::ModuleConfigInfo::
filterAction() const
{
  return filterAction_;
}

inline
std::string const &
art::detail::ModuleConfigInfo::
simpleLabel() const
{
  return simpleLabel_;
}

inline
art::ModuleType
art::detail::ModuleConfigInfo::
moduleType() const
{
  return moduleType_;
}

inline
std::string const &
art::detail::ModuleConfigInfo::
libSpec() const
{
  return libSpec_;
}
#endif /* art_Framework_Core_detail_ModuleConfigInfo_h */

// Local Variables:
// mode: c++
// End:
