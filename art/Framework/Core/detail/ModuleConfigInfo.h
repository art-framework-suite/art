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
                   std::string const & label,
                   std::string const & configPath);

  std::string const & label() const;
  std::string const & configPath() const;
  WorkerInPath::FilterAction filterAction() const;
  ModuleType moduleType() const;
  fhicl::ParameterSet const & modPS() const;
  std::string const & libSpec() const;

  static
  std::vector<std::string> const & allModulePathRoots();

  static
  std::string stripLabel(std::string const & labelInPathConfig);

private:
  WorkerInPath::FilterAction calcFilterAction_() const;
  ModuleType calcConfigType_() const;

  std::string const label_;
  std::string const configPath_;
  WorkerInPath::FilterAction const filterAction_;
  ModuleType const moduleType_;
  fhicl::ParameterSet const modPS_;
  std::string const libSpec_;
};

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
label() const
{
  return label_;
}

inline
art::ModuleType
art::detail::ModuleConfigInfo::
moduleType() const
{
  return moduleType_;
}

inline
fhicl::ParameterSet const &
art::detail::ModuleConfigInfo::
modPS() const
{
  return modPS_;
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
