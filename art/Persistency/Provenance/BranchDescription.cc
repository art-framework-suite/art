#include "art/Persistency/Provenance/BranchDescription.h"

#include "Cintex/Cintex.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Utilities/FriendlyName.h"
#include "art/Utilities/WrappedClassName.h"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSetID.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include <cassert>
#include <cstdlib>
#include <ostream>
#include <sstream>

// FIXME: This should go away as soon as ROOT makes this function
// public. In the meantime, we have to verify that this signature does
// not change in new versions of ROOT.
namespace ROOT {
  namespace Cintex {
    std::string CintName(const std::string&);
  }
}

using fhicl::ParameterSetID;

namespace art {

  BranchDescription::Transients::Transients() :
    parameterSetID_(),
    branchName_(),
    wrappedName_(),
    produced_(false),
    present_(true),
    transient_(false),
    type_(),
    splitLevel_(),
    basketSize_() {
   }

  BranchDescription::BranchDescription() :
    branchType_(InEvent),
    moduleLabel_(),
    processName_(),
    branchID_(),
    fullClassName_(),
    friendlyClassName_(),
    productInstanceName_(),
    psetIDs_(),
    processConfigurationIDs_(),
    branchAliases_(),
    transients_()
  {
    // do not call init here! It will result in an exception throw.
  }

  BranchDescription::BranchDescription(
                        BranchType const& branchType,
                        std::string const& mdLabel,
                        std::string const& procName,
                        std::string const& name,
                        std::string const& fName,
                        std::string const& pin,
                        ModuleDescription const& modDesc,
                        std::set<std::string> const& aliases) :
    branchType_(branchType),
    moduleLabel_(mdLabel),
    processName_(procName),
    branchID_(),
    fullClassName_(name),
    friendlyClassName_(fName),
    productInstanceName_(pin),
    psetIDs_(),
    processConfigurationIDs_(),
    branchAliases_(aliases),
    transients_()
  {
    guts().present_ = true;
    guts().produced_ = true;
    guts().parameterSetID_ = modDesc.parameterSetID();
    psetIDs_.insert(modDesc.parameterSetID());
    processConfigurationIDs_.insert(modDesc.processConfigurationID());
    init();
  }

  void
  BranchDescription::init() const
  {
    if (!guts().branchName_.empty()) return;
    throwIfInvalid_();

    char const underscore('_');
    char const period('.');

    if (friendlyClassName_.find(underscore) != std::string::npos) {
      throw cet::exception("IllegalCharacter") << "Class name '" << friendlyClassName()
      << "' contains an underscore ('_'), which is illegal in the name of a product.\n";
    }

    if (moduleLabel_.find(underscore) != std::string::npos) {
      throw cet::exception("IllegalCharacter") << "Module label '" << moduleLabel()
      << "' contains an underscore ('_'), which is illegal in a module label.\n";
    }

    if (productInstanceName_.find(underscore) != std::string::npos) {
      throw cet::exception("IllegalCharacter") << "Product instance name '" << productInstanceName()
      << "' contains an underscore ('_'), which is illegal in a product instance name.\n";
    }

    if (processName_.find(underscore) != std::string::npos) {
      throw cet::exception("IllegalCharacter") << "Process name '" << processName()
      << "' contains an underscore ('_'), which is illegal in a process name.\n";
    }

    transients_.get().branchName_.reserve(friendlyClassName().size() +
                        moduleLabel().size() +
                        productInstanceName().size() +
                        processName().size() + 4);
    transients_.get().branchName_ += friendlyClassName();
    transients_.get().branchName_ += underscore;
    transients_.get().branchName_ += moduleLabel();
    transients_.get().branchName_ += underscore;
    transients_.get().branchName_ += productInstanceName();
    transients_.get().branchName_ += underscore;
    transients_.get().branchName_ += processName();
    transients_.get().branchName_ += period;

    if (!branchID_.isValid()) {
      branchID_.setID(guts().branchName_);
    }

    Reflex::Type t = Reflex::Type::ByName(fullClassName());
    Reflex::PropertyList p = t.Properties();
    if (p.HasProperty("persistent") && p.PropertyAsString("persistent") == std::string("false")) {
      mf::LogWarning("TransientBranch")
        << "BranchDescription::init() called for the non-persistable entity: "
        << friendlyClassName()
        << ".\nPlease check your experiment's policy on the advisability of such products.\n";
      transients_.get().transient_ = true;
    }

    transients_.get().wrappedName_ = wrappedClassName(fullClassName());
    transients_.get().wrappedCintName_ = wrappedClassName(ROOT::Cintex::CintName(fullClassName()));
    transients_.get().type_ = Reflex::Type::ByName(transients_.get().wrappedName_);
    Reflex::PropertyList wp = guts().type_.Properties();
    if (wp.HasProperty("splitLevel")) {
        transients_.get().splitLevel_ = strtol(wp.PropertyAsString("splitLevel").c_str(), 0, 0);
        if (transients_.get().splitLevel_ < 0) {
          throw cet::exception("IllegalSplitLevel") << "' An illegal ROOT split level of " <<
          transients_.get().splitLevel_ << " is specified for class " << transients_.get().wrappedName_ << ".'\n";
        }
        ++transients_.get().splitLevel_; //Compensate for wrapper
    } else {
        transients_.get().splitLevel_ = invalidSplitLevel;
    }
    if (wp.HasProperty("basketSize")) {
        transients_.get().basketSize_ = strtol(wp.PropertyAsString("basketSize").c_str(), 0, 0);
        if (transients_.get().basketSize_ <= 0) {
          throw cet::exception("IllegalBasketSize") << "' An illegal ROOT basket size of " <<
          transients_.get().basketSize_ << " is specified for class " << transients_.get().wrappedName_ << "'.\n";
        }
    } else {
        transients_.get().basketSize_ = invalidBasketSize;
    }
  }  // init()

  ParameterSetID const&
    BranchDescription::psetID() const {
    assert(!psetIDs().empty());
    if (psetIDs().size() != 1) {
      throw cet::exception("Ambiguous")
        << "Your application requires all events on Branch '" << guts().branchName_
        << "'\n to have the same provenance. This file has events with mixed provenance\n"
        << "on this branch.  Use a different input file.\n";
    }
    return *psetIDs().begin();
  }

  void
  BranchDescription::merge(BranchDescription const& other) {
    psetIDs_.insert(other.psetIDs().begin(), other.psetIDs().end());
    processConfigurationIDs_.insert(other.processConfigurationIDs().begin(), other.processConfigurationIDs().end());
    branchAliases_.insert(other.branchAliases().begin(), other.branchAliases().end());
    guts().present_ = guts().present_ || other.guts().present_;
    if (guts().splitLevel_ == invalidSplitLevel) guts().splitLevel_ = other.guts().splitLevel_;
    if (guts().basketSize_ == invalidBasketSize) guts().basketSize_ = other.guts().basketSize_;
  }

  void
  BranchDescription::write(std::ostream& os) const {
    os << "Branch Type = " << branchType_ << std::endl;
    os << "Process Name = " << processName() << std::endl;
    os << "ModuleLabel = " << moduleLabel() << std::endl;
    os << "Branch ID = " << branchID() << '\n';
    os << "Class Name = " << fullClassName() << '\n';
    os << "Friendly Class Name = " << friendlyClassName() << '\n';
    os << "Product Instance Name = " << productInstanceName() << std::endl;
  }

  void
  BranchDescription::swap(BranchDescription &other) {
    using std::swap;
    swap(branchType_, other.branchType_);
    swap(moduleLabel_, other.moduleLabel_);
    swap(processName_, other.processName_);
    swap(branchID_, other.branchID_);
    swap(fullClassName_, other.fullClassName_);
    swap(friendlyClassName_, other.friendlyClassName_);
    swap(productInstanceName_, other.productInstanceName_);
    swap(psetIDs_, other.psetIDs_);
    swap(processConfigurationIDs_, other.processConfigurationIDs_);
    swap(branchAliases_, other.branchAliases_);
    swap(transients_, other.transients_);
  };

  void throwExceptionWithText(const char* txt)
  {
    throw Exception(errors::LogicError)
      << "Problem using an incomplete BranchDescription\n"
      << txt
      << "\nPlease report this error to the ART developers\n";
  }

  void
  BranchDescription::throwIfInvalid_() const
  {
    if (branchType_ >= art::NumBranchTypes)
      throwExceptionWithText("Illegal BranchType detected");

    if (moduleLabel_.empty())
      throwExceptionWithText("Module label is not allowed to be empty");

    if (processName_.empty())
      throwExceptionWithText("Process name is not allowed to be empty");

    if (fullClassName_.empty())
      throwExceptionWithText("Full class name is not allowed to be empty");

    if (friendlyClassName_.empty())
      throwExceptionWithText("Friendly class name is not allowed to be empty");

    if (produced() && !parameterSetID().is_valid())
      throwExceptionWithText("Invalid ParameterSetID detected");
  }

  void
  BranchDescription::updateFriendlyClassName() {
    friendlyClassName_ = friendlyname::friendlyName(fullClassName());
  }

  bool
  operator<(BranchDescription const& a, BranchDescription const& b) {
    if (a.processName() < b.processName()) return true;
    if (b.processName() < a.processName()) return false;
    if (a.fullClassName() < b.fullClassName()) return true;
    if (b.fullClassName() < a.fullClassName()) return false;
    if (a.friendlyClassName() < b.friendlyClassName()) return true;
    if (b.friendlyClassName() < a.friendlyClassName()) return false;
    if (a.productInstanceName() < b.productInstanceName()) return true;
    if (b.productInstanceName() < a.productInstanceName()) return false;
    if (a.moduleLabel() < b.moduleLabel()) return true;
    if (b.moduleLabel() < a.moduleLabel()) return false;
    if (a.branchType() < b.branchType()) return true;
    if (b.branchType() < a.branchType()) return false;
    if (a.branchID() < b.branchID()) return true;
    if (b.branchID() < a.branchID()) return false;
    if (a.psetIDs() < b.psetIDs()) return true;
    if (b.psetIDs() < a.psetIDs()) return false;
    if (a.processConfigurationIDs() < b.processConfigurationIDs()) return true;
    if (b.processConfigurationIDs() < a.processConfigurationIDs()) return false;
    if (a.branchAliases() < b.branchAliases()) return true;
    if (b.branchAliases() < a.branchAliases()) return false;
    if (a.present() < b.present()) return true;
    if (b.present() < a.present()) return false;
    return false;
  }

  bool
  combinable(BranchDescription const& a, BranchDescription const& b) {
    return
    (a.branchType() == b.branchType()) &&
    (a.processName() == b.processName()) &&
    (a.fullClassName() == b.fullClassName()) &&
    (a.friendlyClassName() == b.friendlyClassName()) &&
    (a.productInstanceName() == b.productInstanceName()) &&
    (a.moduleLabel() == b.moduleLabel()) &&
    (a.branchID() == b.branchID());
  }

  bool
  operator==(BranchDescription const& a, BranchDescription const& b) {
    return combinable(a, b) &&
      (a.present() == b.present()) &&
       (a.psetIDs() == b.psetIDs()) &&
       (a.processConfigurationIDs() == b.processConfigurationIDs()) &&
       (a.branchAliases() == b.branchAliases());
  }

  std::string
  match(BranchDescription const& a, BranchDescription const& b,
        std::string const& fileName,
        BranchDescription::MatchMode m) {
    std::ostringstream differences;
    if (a.branchName() != b.branchName()) {
      differences << "Branch name '" << b.branchName() << "' does not match '" << a.branchName() << "'.\n";
      // Need not compare components of branch name individually.
      // (a.friendlyClassName() != b.friendlyClassName())
      // (a.moduleLabel() != b.moduleLabel())
      // (a.productInstanceName() != b.productInstanceName())
      // (a.processName() != b.processName())
    }
    if (a.branchType() != b.branchType()) {
      differences << "Branch '" << b.branchName() << "' is a(n) '" << b.branchType() << "' branch\n";
      differences << "    in file '" << fileName << "', but a(n) '" << a.branchType() << "' branch in previous files.\n";
    }
    if (a.branchID() != b.branchID()) {
      differences << "Branch '" << b.branchName() << "' has a branch ID of '" << b.branchID() << "'\n";
      differences << "    in file '" << fileName << "', but '" << a.branchID() << "' in previous files.\n";
    }
    if (a.fullClassName() != b.fullClassName()) {
      differences << "Products on branch '" << b.branchName() << "' have type '" << b.fullClassName() << "'\n";
      differences << "    in file '" << fileName << "', but '" << a.fullClassName() << "' in previous files.\n";
    }
    if (b.present() && !a.present()) {
      differences << "Branch '" << a.branchName() << "' was dropped in previous files but is present in '" << fileName << "'.\n";
    }
    if (m == BranchDescription::Strict) {
        if (b.psetIDs().size() > 1) {
          differences << "Branch '" << b.branchName() << "' uses more than one parameter set in file '" << fileName << "'.\n";
        } else if (a.psetIDs().size() > 1) {
          differences << "Branch '" << a.branchName() << "' uses more than one parameter set in previous files.\n";
        } else if (a.psetIDs() != b.psetIDs()) {
          differences << "Branch '" << b.branchName() << "' uses different parameter sets in file '" << fileName << "'.\n";
          differences << "    than in previous files.\n";
        }

        if (b.processConfigurationIDs().size() > 1) {
          differences << "Branch '" << b.branchName() << "' uses more than one process configuration in file '" << fileName << "'.\n";
        } else if (a.processConfigurationIDs().size() > 1) {
          differences << "Branch '" << a.branchName() << "' uses more than one process configuration in previous files.\n";
        } else if (a.processConfigurationIDs() != b.processConfigurationIDs()) {
          differences << "Branch '" << b.branchName() << "' uses different process configurations in file '" << fileName << "'.\n";
          differences << "    than in previous files.\n";
        }
    }
    return differences.str();
  }
}
