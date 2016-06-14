#include "art/Persistency/Provenance/ModuleDescription.h"

#include "cetlib/MD5Digest.h"
#include <ostream>
#include <sstream>

/*----------------------------------------------------------------------



----------------------------------------------------------------------*/

namespace art {

  ModuleDescription::ModuleDescription() :
    parameterSetID_(),
    moduleName_(),
    moduleLabel_(),
    processConfiguration_()
  { }

  ModuleDescription::ModuleDescription(fhicl::ParameterSetID parameterSetID,
                                       std::string const & modName,
                                       std::string const & modLabel,
                                       ProcessConfiguration pc)
  :
    parameterSetID_(parameterSetID),
    moduleName_(modName),
    moduleLabel_(modLabel),
    processConfiguration_(std::move(pc))
  {
  }

  bool
  ModuleDescription::operator<(ModuleDescription const& rh) const {
    if (moduleLabel() < rh.moduleLabel()) return true;
    if (rh.moduleLabel() < moduleLabel()) return false;
    if (processName() < rh.processName()) return true;
    if (rh.processName() < processName()) return false;
    if (moduleName() < rh.moduleName()) return true;
    if (rh.moduleName() < moduleName()) return false;
    if (parameterSetID() < rh.parameterSetID()) return true;
    if (rh.parameterSetID() < parameterSetID()) return false;
    if (releaseVersion() < rh.releaseVersion()) return true;
    if (rh.releaseVersion() < releaseVersion()) return false;
    if (passID() < rh.passID()) return true;
    return false;
  }

  bool
  ModuleDescription::operator==(ModuleDescription const& rh) const {
    return !((*this) < rh || rh < (*this));
  }

  bool
  ModuleDescription::operator!=(ModuleDescription const& rh) const {
    return !((*this) == rh);
  }

  void
  ModuleDescription::write(std::ostream& os) const {
    os  << "Module type=" << moduleName() << ", "
        << "Module label=" << moduleLabel() << ", "
        << "Parameter Set ID=" << parameterSetID() << ", "
        << "Process name=" << processName() << ", "
        << "Release Version=" << releaseVersion() << ", "
        << "Pass ID=" << passID() << ", "
        << "Main Parameter Set ID=" << mainParameterSetID();
  }

  ModuleDescriptionID
  ModuleDescription::id() const
  {
    // This implementation is ripe for optimization.
    // We do not use operator<< because it does not write out everything.
    std::ostringstream oss;
    oss << parameterSetID() << ' '
        << moduleName() << ' '
        << moduleLabel() << ' '
        << mainParameterSetID() << ' '
        << releaseVersion() << ' '
        << processName() << ' '
        << passID();
    std::string stringrep = oss.str();
    cet::MD5Digest md5alg(stringrep);
    return ModuleDescriptionID(md5alg.digest().toString());
  }

}
