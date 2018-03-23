#include "art/Framework/IO/Root/InitRootHandlers.h"
#include "art/Framework/IO/Root/RootDB/tkeyvfs.h"
#include "canvas/Utilities/Exception.h"
#include "canvas_root_io/Streamers/BranchDescriptionStreamer.h"
#include "canvas_root_io/Streamers/CacheStreamers.h"
#include "canvas_root_io/Streamers/ProductIDStreamer.h"
#include "canvas_root_io/Streamers/RefCoreStreamer.h"
#include "canvas_root_io/Streamers/TransientStreamer.h"
#include "canvas_root_io/Streamers/setPtrVectorBaseStreamer.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "TError.h"
#include "TH1.h"
#include "TROOT.h"
#include "TSystem.h"
#include "TTree.h"

#include <sstream>
#include <string.h>

namespace {

  enum class SeverityLevel { kInfo, kWarning, kError, kSysError, kFatal };

  void
  RootErrorHandler(int const level,
                   bool die,
                   char const* location,
                   char const* message)
  {
    using mf::ELseverityLevel;
    auto const npos = std::string::npos;

    // Translate ROOT severity level to MessageLogger severity level

    SeverityLevel el_severity{SeverityLevel::kInfo};
    if (level >= kFatal) {
      el_severity = SeverityLevel::kFatal;
    } else if (level >= kSysError) {
      el_severity = SeverityLevel::kSysError;
    } else if (level >= kError) {
      el_severity = SeverityLevel::kError;
    } else if (level >= kWarning) {
      el_severity = SeverityLevel::kWarning;
    }

    // Adapt C-strings to std::strings
    // Arrange to report the error location as furnished by Root

    std::string el_location{"@SUB=?"};
    if (location != nullptr) {
      el_location = "@SUB=" + std::string(location);
    }

    std::string el_message{"?"};
    if (message != nullptr) {
      el_message = message;
    }

    // Try to create a meaningful id string using knowledge of ROOT error
    // messages
    //
    // id ==     "ROOT-ClassName" where ClassName is the affected class
    //      else "ROOT/ClassName" where ClassName is the error-declaring class
    //      else "ROOT"

    std::string el_identifier{"ROOT"};

    std::string const precursor{"class "};
    size_t index1 = el_message.find(precursor);
    if (index1 != npos) {
      size_t index2 = index1 + precursor.length();
      size_t index3 = el_message.find_first_of(" :", index2);
      if (index3 != npos) {
        size_t substrlen = index3 - index2;
        el_identifier += "-";
        el_identifier += el_message.substr(index2, substrlen);
      }
    } else {
      index1 = el_location.find("::");
      if (index1 != npos) {
        el_identifier += "/";
        el_identifier += el_location.substr(0, index1);
      }
    }

    // Intercept and ignore messages:
    for (std::string const& s :
         {"art::Transient<art::ProductRegistry::Transients>",
          "art::DoNotRecordParents"}) {
      if (el_message ==
          (std::string("no dictionary for class ") + s + " is available")) {
        return;
      }
    }

    // Intercept some messages and upgrade the severity
    if ((el_location.find("TBranchElement::Fill") != npos) &&
        (el_message.find("fill branch") != npos) &&
        (el_message.find("address") != npos) &&
        (el_message.find("not set") != npos)) {
      el_severity = SeverityLevel::kFatal;
    }
    if ((el_message.find("Tree branches") != npos) &&
        (el_message.find("different numbers of entries") != npos)) {
      el_severity = SeverityLevel::kFatal;
    }

    // Intercept some messages and downgrade the severity
    if ((el_message.find("dictionary") != npos) ||
        (el_message.find("already in TClassTable") != npos) ||
        (el_message.find("matrix not positive definite") != npos) ||
        (el_message.find("number of iterations was insufficient") != npos) ||
        (el_message.find("bad integrand behavior") != npos) ||
        (el_location.find("Fit") != npos) ||
        (el_location.find("TDecompChol::Solve") != npos) ||
        (el_location.find("THistPainter::PaintInit") != npos) ||
        (el_location.find("TGClient::GetFontByName") != npos)) {
      el_severity = SeverityLevel::kInfo;
    }

    if ((el_location.find("TUnixSystem::SetDisplay") != npos) &&
        (el_message.find("DISPLAY not set") != npos)) {
      el_severity = SeverityLevel::kInfo;
    }

    if ((el_location.find("TTree::ReadStream") != npos) &&
        (el_message.find("Ignoring trailing") == 0)) {
      el_severity = SeverityLevel::kInfo;
    }

    if (el_severity == SeverityLevel::kInfo) {
      // Don't throw if the message is just informational.
      die = false;
    } else {
      die = true;
    }

    // Feed the message to the MessageLogger and let it choose to suppress or
    // not.

    // Root has declared a fatal error.  Throw an exception unless the
    // message corresponds to a pending signal. In that case, do not
    // throw but let the OS deal with the signal in the usual way.
    if (die && (location != std::string("TUnixSystem::DispatchSignals"))) {
      std::ostringstream sstr;
      sstr << "Fatal Root Error: " << el_location << "\n" << el_message << '\n';
      throw art::Exception{art::errors::FatalRootError, sstr.str()};
    }

    // Currently we get here only for informational messages,
    // but we leave the other code in just in case we change
    // the criteria for throwing.
    if (el_severity == SeverityLevel::kFatal) {
      mf::LogError("Root_Fatal") << el_location << el_message;
    } else if (el_severity == SeverityLevel::kSysError) {
      mf::LogError("Root_Severe") << el_location << el_message;
    } else if (el_severity == SeverityLevel::kError) {
      mf::LogError("Root_Error") << el_location << el_message;
    } else if (el_severity == SeverityLevel::kWarning) {
      mf::LogWarning("Root_Warning") << el_location << el_message;
    } else if (el_severity == SeverityLevel::kInfo) {
      mf::LogInfo("Root_Information") << el_location << el_message;
    }
  }
} // namespace

namespace art {

  void
  unloadRootSigHandler()
  {
    // Deactivate all the Root signal handlers and restore the system defaults
    gSystem->ResetSignal(kSigChild);
    gSystem->ResetSignal(kSigBus);
    gSystem->ResetSignal(kSigSegmentationViolation);
    gSystem->ResetSignal(kSigIllegalInstruction);
    gSystem->ResetSignal(kSigSystem);
    gSystem->ResetSignal(kSigPipe);
    gSystem->ResetSignal(kSigAlarm);
    gSystem->ResetSignal(kSigUrgent);
    gSystem->ResetSignal(kSigFloatingException);
    gSystem->ResetSignal(kSigWindowChanged);
  }

  void
  setRootErrorHandler(bool const want_custom)
  {
    if (want_custom) {
      SetErrorHandler(RootErrorHandler);
    } else {
      SetErrorHandler(DefaultErrorHandler);
    }
  }

  void
  completeRootHandlers()
  {
    // Set ROOT parameters.
    ROOT::EnableThreadSafety();
    TTree::SetMaxTreeSize(kMaxLong64);
    TH1::AddDirectory(kFALSE);

    // Initialize tkeyvfs sqlite3 extension for ROOT.
    tkeyvfs_init();

    // Set custom streamers.
    setCacheStreamers();
    setProvenanceTransientStreamers();
    detail::setBranchDescriptionStreamer();
    detail::setPtrVectorBaseStreamer();
    configureProductIDStreamer();
    configureRefCoreStreamer();
  }

} // namespace art
