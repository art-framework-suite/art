#ifndef art_Persistency_Provenance_ModuleDescription_h
#define art_Persistency_Provenance_ModuleDescription_h
// vim: set sw=2 expandtab :

#include "art/Persistency/Provenance/ModuleType.h"
#include "canvas/Persistency/Provenance/ProcessConfiguration.h"
#include "canvas/Persistency/Provenance/ProcessConfigurationID.h"
#include "fhiclcpp/ParameterSetID.h"
#include <iosfwd>
#include <string>

namespace art {

  class ModuleDescription {
  public:
    ~ModuleDescription();
    explicit ModuleDescription();
    explicit ModuleDescription(fhicl::ParameterSetID parameterSetID,
                               std::string const& modName,
                               std::string const& modLabel,
                               ModuleThreadingType moduleThreadingType,
                               ProcessConfiguration pc,
                               bool isEmulated = false);

    void write(std::ostream& os) const;

    fhicl::ParameterSetID const& parameterSetID() const;
    std::string const& moduleName() const;
    std::string const& moduleLabel() const;
    bool isEmulatedModule() const;

    ModuleThreadingType moduleThreadingType() const;

    ProcessConfiguration const& processConfiguration() const;
    ProcessConfigurationID const processConfigurationID() const;
    std::string const& processName() const;
    std::string const& releaseVersion() const;
    fhicl::ParameterSetID const& mainParameterSetID() const;

    bool operator<(ModuleDescription const& rh) const;
    bool operator==(ModuleDescription const& rh) const;
    bool operator!=(ModuleDescription const& rh) const;

  private:
    // Properties of the product creator
    fhicl::ParameterSetID parameterSetID_{};
    std::string moduleName_{}; // class name
    std::string moduleLabel_{};
    ModuleThreadingType moduleThreadingType_{};
    bool isEmulated_{false};

    // Process-wide configuration
    ProcessConfiguration processConfiguration_{"invalid_process",
                                               fhicl::ParameterSetID{},
                                               ReleaseVersion{}};
  };

  std::ostream& operator<<(std::ostream& os, ModuleDescription const& p);

} // namespace art

#endif /* art_Persistency_Provenance_ModuleDescription_h */

// Local Variables:
// mode: c++
// End:
