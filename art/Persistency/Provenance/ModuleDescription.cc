#include "art/Persistency/Provenance/ModuleDescription.h"
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Provenance/ProcessConfiguration.h"
#include "fhiclcpp/ParameterSetID.h"

#include <ostream>

using namespace std;

namespace art {

  ModuleDescription::~ModuleDescription() = default;
  ModuleDescription::ModuleDescription() = default;

  ModuleDescription::ModuleDescription(
    fhicl::ParameterSetID const parameterSetID,
    string const& modName,
    string const& modLabel,
    ModuleThreadingType const moduleThreadingType,
    ProcessConfiguration pc,
    bool const isEmulated)
    : parameterSetID_{parameterSetID}
    , moduleName_{modName}
    , moduleLabel_{modLabel}
    , moduleThreadingType_{moduleThreadingType}
    , isEmulated_{isEmulated}
    , processConfiguration_{std::move(pc)}
  {}

  fhicl::ParameterSetID const&
  ModuleDescription::parameterSetID() const
  {
    return parameterSetID_;
  }

  std::string const&
  ModuleDescription::moduleName() const
  {
    return moduleName_;
  }

  std::string const&
  ModuleDescription::moduleLabel() const
  {
    return moduleLabel_;
  }

  bool
  ModuleDescription::isEmulatedModule() const
  {
    return isEmulated_;
  }

  ModuleThreadingType
  ModuleDescription::moduleThreadingType() const
  {
    return moduleThreadingType_;
  }

  ProcessConfiguration const&
  ModuleDescription::processConfiguration() const
  {
    return processConfiguration_;
  }

  ProcessConfigurationID const
  ModuleDescription::processConfigurationID() const
  {
    return processConfiguration().id();
  }

  std::string const&
  ModuleDescription::processName() const
  {
    return processConfiguration().processName();
  }

  std::string const&
  ModuleDescription::releaseVersion() const
  {
    return processConfiguration().releaseVersion();
  }

  fhicl::ParameterSetID const&
  ModuleDescription::mainParameterSetID() const
  {
    return processConfiguration().parameterSetID();
  }

  bool
  ModuleDescription::operator<(ModuleDescription const& rh) const
  {
    if (moduleLabel() < rh.moduleLabel()) {
      return true;
    }
    if (rh.moduleLabel() < moduleLabel()) {
      return false;
    }
    if (processName() < rh.processName()) {
      return true;
    }
    if (rh.processName() < processName()) {
      return false;
    }
    if (moduleName() < rh.moduleName()) {
      return true;
    }
    if (rh.moduleName() < moduleName()) {
      return false;
    }
    if (parameterSetID() < rh.parameterSetID()) {
      return true;
    }
    if (rh.parameterSetID() < parameterSetID()) {
      return false;
    }
    if (releaseVersion() < rh.releaseVersion()) {
      return true;
    }
    return false;
  }

  bool
  ModuleDescription::operator==(ModuleDescription const& rh) const
  {
    return !((*this < rh) || (rh < *this));
  }

  bool
  ModuleDescription::operator!=(ModuleDescription const& rh) const
  {
    return !operator==(rh);
  }

  void
  ModuleDescription::write(ostream& os) const
  {
    os << "Module type=" << moduleName() << ", "
       << "Module label=" << moduleLabel() << ", "
       << "Parameter Set ID=" << parameterSetID() << ", "
       << "Process name=" << processName() << ", "
       << "Release Version=" << releaseVersion() << ", "
       << "Main Parameter Set ID=" << mainParameterSetID();
  }

  std::ostream&
  operator<<(std::ostream& os, ModuleDescription const& p)
  {
    p.write(os);
    return os;
  }

} // namespace art
