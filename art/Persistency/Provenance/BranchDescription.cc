#include "art/Persistency/Provenance/BranchDescription.h"

#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/FriendlyName.h"
#include "art/Utilities/WrappedClassName.h"
#include "fhiclcpp/ParameterSetID.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "Compression.h"
#include "TDictAttributeMap.h"

#include "TBuffer.h"
#include "TClass.h"
#include "TClassStreamer.h" // Temporary

#include <cassert>
#include <cstdlib>
#include <ostream>
#include <sstream>

using fhicl::ParameterSetID;

namespace {
  void throwExceptionWithText(const char* txt)
  {
    throw art::Exception(art::errors::LogicError)
      << "Problem using an incomplete BranchDescription\n"
      << txt
      << "\nPlease report this error to the ART developers\n";
  }

  static char const underscore('_');
  static char const period('.');

}

art::BranchDescription::Transients::Transients() :
  branchName_(),
  wrappedName_(),
  produced_(false),
  transient_(false),
  splitLevel_(),
  basketSize_(),
  compression_(invalidCompression)
{
}

art::BranchDescription::BranchDescription() :
  branchType_(InEvent),
  moduleLabel_(),
  processName_(),
  branchID_(),
  producedClassName_(),
  friendlyClassName_(),
  productInstanceName_(),
  psetIDs_(),
  processConfigurationIDs_(),
  transients_()
{
}

art::BranchDescription::
BranchDescription(TypeLabel const &tl,
                  ModuleDescription const& md) :
  branchType_(tl.branchType),
  moduleLabel_(tl.hasEmulatedModule() ? tl.emulatedModule : md.moduleLabel()),
  processName_(md.processName()),
  branchID_(),
  producedClassName_(tl.className()),
  friendlyClassName_(tl.friendlyClassName()),
  productInstanceName_(tl.productInstanceName),
  psetIDs_(),
  processConfigurationIDs_(),
  transients_()
{
  guts().produced_ = true;
  psetIDs_.insert(md.parameterSetID());
  processConfigurationIDs_.insert(md.processConfigurationID());
  throwIfInvalid_();
  fluffTransients_();
  initBranchID_();
}

void art::BranchDescription::initBranchID_() {
  if (!transientsFluffed_()) return;
  if (!branchID_.isValid()) {
    branchID_.setID(guts().branchName_);
  }
}

void art::BranchDescription::fluffTransients_() const {
  if (transientsFluffed_()) return;

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
  // It is *absolutely* needed to have the trailing period on the branch
  // name, as this gives instruction to ROOT to split this branch in the
  // modern (v4+) way vs the old way (v3-).

  TDictAttributeMap & pp = *TClass::GetClass(producedClassName().c_str())->GetAttributeMap();
  if (pp.HasKey("persistent") &&
      pp.GetPropertyAsString("persistent") == std::string("false")) {
    mf::LogWarning("TransientBranch")
      << "BranchDescription::fluffTransients() called for the non-persistable\n"
      << "entity: "
      << friendlyClassName()
      << ". Please check your experiment's policy\n"
      << "on the advisability of such products.\n";
    transients_.get().transient_ = true;
  }

  transients_.get().wrappedName_ = wrappedClassName(producedClassName());
  TDictAttributeMap & wp = *TClass::GetClass(transients_.get().wrappedName_.c_str())->GetAttributeMap();
  if (wp.HasKey("splitLevel")) {
    transients_.get().splitLevel_ =
      strtol(wp.GetPropertyAsString("splitLevel"), 0, 0);
    if (transients_.get().splitLevel_ < 0) {
      throw Exception(errors::Configuration, "IllegalSplitLevel")
        << "' An illegal ROOT split level of "
        << transients_.get().splitLevel_
        << " is specified for class "
        << transients_.get().wrappedName_
        << ".'\n";
    }
    ++transients_.get().splitLevel_; //Compensate for wrapper
  } else {
    transients_.get().splitLevel_ = invalidSplitLevel;
  }
  if (wp.HasKey("basketSize")) {
    transients_.get().basketSize_ =
      strtol(wp.GetPropertyAsString("basketSize"), 0, 0);
    if (transients_.get().basketSize_ <= 0) {
      throw Exception(errors::Configuration, "IllegalBasketSize")
        << "' An illegal ROOT basket size of "
        << transients_.get().basketSize_
        << " is specified for class "
        << transients_.get().wrappedName_
        << "'.\n";
    }
  } else {
    transients_.get().basketSize_ = invalidBasketSize;
  }
  if (wp.HasKey("compression")) {
    // FIXME: We need to check for a parsing error from the strtol() here!
    int compression = strtol(wp.GetPropertyAsString("compression"), 0, 0);
    if (compression < 0) {
      throw Exception(errors::Configuration, "IllegalCompression")
        << "' An illegal ROOT compression of "
        << compression
        << " is specified for class "
        << transients_.get().wrappedName_
        << "'.\n";
    }
    int algorithm = compression / 100;
    int level = compression % 100;
    if (algorithm >= ROOT::kUndefinedCompressionAlgorithm) {
      throw Exception(errors::Configuration, "IllegalCompressionAlgorithm")
        << "' An illegal ROOT compression algorithm of "
        << algorithm
        << " is specified for class "
        << transients_.get().wrappedName_
        << "'.\n";
    }
    if (level > 9) {
      throw Exception(errors::Configuration, "IllegalCompressionLevel")
        << "' An illegal ROOT compression level of "
        << algorithm
        << " is specified for class "
        << transients_.get().wrappedName_
        << "'.  The compression level must between 0 and 9 inclusive.\n";
    }
    transients_.get().compression_ = compression;
  }
}

ParameterSetID const&
art::BranchDescription::psetID() const {
  assert(!psetIDs().empty());
  if (psetIDs().size() != 1) {
    throw Exception(errors::Configuration, "AmbiguousProvenance")
      << "Your application requires all events on Branch '"
      << guts().branchName_
      << "'\n to have the same provenance. This file has events with mixed provenance\n"
      << "on this branch.  Use a different input file.\n";
  }
  return *psetIDs().begin();
}

void
art::BranchDescription::merge(BranchDescription const& other) {
  psetIDs_.insert(other.psetIDs().begin(), other.psetIDs().end());
  processConfigurationIDs_.insert(other.processConfigurationIDs().begin(),
                                  other.processConfigurationIDs().end());
  if (guts().splitLevel_ == invalidSplitLevel) {
    guts().splitLevel_ = other.guts().splitLevel_;
  }
  if (guts().basketSize_ == invalidBasketSize) {
    guts().basketSize_ = other.guts().basketSize_;
  }
  // FIXME: This is probably wrong! We are going from defaulted compression
  //        to a possibly different compression, bad.
  if (guts().compression_ == invalidCompression) {
    guts().compression_ = other.guts().compression_;
  }
}

void
art::BranchDescription::write(std::ostream& os) const {
  os << "Branch Type = " << branchType_ << std::endl;
  os << "Process Name = " << processName() << std::endl;
  os << "ModuleLabel = " << moduleLabel() << std::endl;
  os << "Branch ID = " << branchID() << '\n';
  os << "Class Name = " << producedClassName() << '\n';
  os << "Friendly Class Name = " << friendlyClassName() << '\n';
  os << "Product Instance Name = " << productInstanceName() << std::endl;
}

void
art::BranchDescription::swap(BranchDescription &other) {
  using std::swap;
  swap(branchType_, other.branchType_);
  swap(moduleLabel_, other.moduleLabel_);
  swap(processName_, other.processName_);
  swap(branchID_, other.branchID_);
  swap(producedClassName_, other.producedClassName_);
  swap(friendlyClassName_, other.friendlyClassName_);
  swap(productInstanceName_, other.productInstanceName_);
  swap(psetIDs_, other.psetIDs_);
  swap(processConfigurationIDs_, other.processConfigurationIDs_);
  swap(transients_, other.transients_);
}

void
art::BranchDescription::throwIfInvalid_() const
{
  if (transientsFluffed_()) return;
  if (branchType_ >= art::NumBranchTypes)
    throwExceptionWithText("Illegal BranchType detected");

  if (moduleLabel_.empty())
    throwExceptionWithText("Module label is not allowed to be empty");

  if (processName_.empty())
    throwExceptionWithText("Process name is not allowed to be empty");

  if (producedClassName_.empty())
    throwExceptionWithText("Full class name is not allowed to be empty");

  if (friendlyClassName_.empty())
    throwExceptionWithText("Friendly class name is not allowed to be empty");

  if (friendlyClassName_.find(underscore) != std::string::npos) {
    throw Exception(errors::LogicError, "IllegalCharacter")
      << "Class name '"
      << friendlyClassName()
      << "' contains an underscore ('_'), which is illegal in the "
      << "name of a product.\n";
  }

  if (moduleLabel_.find(underscore) != std::string::npos) {
    throw Exception(errors::Configuration, "IllegalCharacter")
      << "Module label '"
      << moduleLabel()
      << "' contains an underscore ('_'), which is illegal in a "
      << "module label.\n";
  }

  if (productInstanceName_.find(underscore) != std::string::npos) {
    throw Exception(errors::Configuration, "IllegalCharacter")
      << "Product instance name '"
      << productInstanceName()
      << "' contains an underscore ('_'), which is illegal in a "
      << "product instance name.\n";
  }

  if (processName_.find(underscore) != std::string::npos) {
    throw Exception(errors::Configuration, "IllegalCharacter")
      << "Process name '"
      << processName()
      << "' contains an underscore ('_'), which is illegal in a "
      << "process name.\n";
  }
}

void
art::BranchDescription::updateFriendlyClassName() {
  friendlyClassName_ = friendlyname::friendlyName(producedClassName());
}

bool
art::operator<(BranchDescription const& a, BranchDescription const& b) {
  if (a.processName() < b.processName()) return true;
  if (b.processName() < a.processName()) return false;
  if (a.producedClassName() < b.producedClassName()) return true;
  if (b.producedClassName() < a.producedClassName()) return false;
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
  return false;
}

bool
art::  combinable(BranchDescription const& a, BranchDescription const& b) {
  return
    (a.branchType() == b.branchType()) &&
    (a.processName() == b.processName()) &&
    (a.producedClassName() == b.producedClassName()) &&
    (a.friendlyClassName() == b.friendlyClassName()) &&
    (a.productInstanceName() == b.productInstanceName()) &&
    (a.moduleLabel() == b.moduleLabel()) &&
    (a.branchID() == b.branchID());
}

bool
art::operator==(BranchDescription const& a, BranchDescription const& b) {
  return combinable(a, b) &&
    (a.psetIDs() == b.psetIDs()) &&
    (a.processConfigurationIDs() == b.processConfigurationIDs());
}

class art::detail::BranchDescriptionStreamer : public TClassStreamer {
public:
  void operator()(TBuffer &R_b, void *objp);
};

void
art::detail::BranchDescriptionStreamer::operator()(TBuffer &R_b, void *objp) {
  static TClassRef cl(TClass::GetClass(typeid(BranchDescription)));
  BranchDescription *obj(reinterpret_cast<BranchDescription*>(objp));
  if (R_b.IsReading()) {
    cl->ReadBuffer(R_b, obj);
    obj->fluffTransients_();
  } else {
    cl->WriteBuffer(R_b, obj);
  }
}

void
art::detail::setBranchDescriptionStreamer() {
  static TClassRef cl(TClass::GetClass(typeid(BranchDescription)));
  if (cl->GetStreamer() == 0) {
    cl->AdoptStreamer(new BranchDescriptionStreamer);
  }
}

std::string
art::match(BranchDescription const& a,
           BranchDescription const& b,
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
  if (a.producedClassName() != b.producedClassName()) {
    differences << "Products on branch '" << b.branchName() << "' have type '" << b.producedClassName() << "'\n";
    differences << "    in file '" << fileName << "', but '" << a.producedClassName() << "' in previous files.\n";
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

std::ostream&
  art::operator<<(std::ostream& os, BranchDescription const& p)
{
  p.write(os);
  return os;
}

