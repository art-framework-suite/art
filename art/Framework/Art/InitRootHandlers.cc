
#include "art/Framework/Art/InitRootHandlers.h"
#include "art/Persistency/Common/CacheStreamers.h"
#include "art/Persistency/Common/detail/setPtrVectorBaseStreamer.h"
#include "art/Framework/IO/Root/RefCoreStreamer.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/TransientStreamer.h"
#include "art/Persistency/RootDB/tkeyvfs.h"

#include "art/Utilities/Exception.h"

#include "messagefacility/MessageLogger/MessageLogger.h"

#include "Cintex/Cintex.h"
#include "G__ci.h"
#include "TError.h"
#include "TH1.h"
#include "TSystem.h"
#include "TTree.h"

#include <sstream>
#include <string.h>

namespace {

  void RootErrorHandler(int level, bool die,
                        char const* location, char const* message)
  {
    using mf::ELseverityLevel;

    // Translate ROOT severity level to MessageLogger severity level

    ELseverityLevel              el_severity = ELseverityLevel::ELsev_info;
    if     ( level >= kFatal   ) el_severity = ELseverityLevel::ELsev_fatal;
    else if( level >= kSysError) el_severity = ELseverityLevel::ELsev_severe;
    else if( level >= kError   ) el_severity = ELseverityLevel::ELsev_error;
    else if( level >= kWarning ) el_severity = ELseverityLevel::ELsev_warning;

    // Adapt C-strings to std::strings
    // Arrange to report the error location as furnished by Root

    std::string el_location = "@SUB=?";
    if( location != 0) el_location = std::string("@SUB=")+std::string(location);

    std::string el_message  = "?";
    if( message != 0)  el_message  = message;

    // Try to create a meaningful id string using knowledge of ROOT error messages
    //
    // id ==     "ROOT-ClassName" where ClassName is the affected class
    //      else "ROOT/ClassName" where ClassName is the error-declaring class
    //      else "ROOT"

    std::string el_identifier = "ROOT";

    std::string precursor("class ");
    size_t index1 = el_message.find(precursor);
    if( index1 != std::string::npos ) {
      size_t index2 = index1 + precursor.length();
      size_t index3 = el_message.find_first_of(" :", index2);
      if( index3 != std::string::npos ) {
        size_t substrlen = index3-index2;
        el_identifier += "-";
        el_identifier += el_message.substr(index2,substrlen);
      }
    }
    else {
      index1 = el_location.find("::");
      if( index1 != std::string::npos ) {
        el_identifier += "/";
        el_identifier += el_location.substr(0, index1);
      }
    }

    // Intercept one message and ignore:
    if (el_message ==
        "no dictionary for class art::Transient<art::ProductRegistry::Transients> is available")
    {
      return;
    }

    // Intercept some messages and upgrade the severity

    if( (el_location.find("TBranchElement::Fill") != std::string::npos)
     && (el_message.find("fill branch") != std::string::npos)
     && (el_message.find("address") != std::string::npos)
     && (el_message.find("not set") != std::string::npos) ) {
      el_severity = ELseverityLevel::ELsev_fatal;
    }
    if( (el_message.find("Tree branches") != std::string::npos)
     && (el_message.find("different numbers of entries") != std::string::npos) ) {
      el_severity = ELseverityLevel::ELsev_fatal;
    }

    // Intercept some messages and downgrade the severity

    if( (el_message.find("dictionary") != std::string::npos) ||
        (el_message.find("already in TClassTable") != std::string::npos) ||
        (el_message.find("matrix not positive definite") != std::string::npos) ||
        (el_location.find("Fit") != std::string::npos) ||
        (el_location.find("TDecompChol::Solve") != std::string::npos) ||
        (el_location.find("THistPainter::PaintInit") != std::string::npos) ||
        (el_location.find("TGClient::GetFontByName") != std::string::npos)) {
      el_severity = ELseverityLevel::ELsev_info;
    }

    if ( (el_location.find("TUnixSystem::SetDisplay") != std::string::npos) &&
         (el_message.find("DISPLAY not set") != std::string::npos) ) {
      el_severity = ELseverityLevel::ELsev_info;
    }

    if( el_severity == ELseverityLevel::ELsev_info ) {
      // Don't throw if the message is just informational.
      die = false;
    } else {
      die = true;
    }

    // Feed the message to the MessageLogger and let it choose to suppress or not.

    // Root has declared a fatal error.  Throw an EDMException unless the
    // message corresponds to a pending signal. In that case, do not throw
    // but let the OS deal with the signal in the usual way.
    if( die && (location != std::string("TUnixSystem::DispatchSignals")) ) {
       std::ostringstream sstr;
       sstr << "Fatal Root Error: " << el_location << "\n" << el_message << '\n';
       art::Exception except(art::errors::FatalRootError, sstr.str());
       throw except;
    }

    // Currently we get here only for informational messages,
    // but we leave the other code in just in case we change
    // the criteria for throwing.
    if( el_severity == ELseverityLevel::ELsev_fatal ) {
      mf::LogError("Root_Fatal") << el_location << el_message;
    }
    else if( el_severity == ELseverityLevel::ELsev_severe ) {
      mf::LogError("Root_Severe") << el_location << el_message;
    }
    else if( el_severity == ELseverityLevel::ELsev_error ) {
      mf::LogError("Root_Error") << el_location << el_message;
    }
    else if( el_severity == ELseverityLevel::ELsev_warning ) {
      mf::LogWarning("Root_Warning") << el_location << el_message ;
    }
    else if( el_severity == ELseverityLevel::ELsev_info ) {
      mf::LogInfo("Root_Information") << el_location << el_message ;
    }
  }

}  // namespace

namespace art {

   void unloadRootSigHandler()
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

   void setRootErrorHandler(bool want_custom)
   {
      if (want_custom) {
         SetErrorHandler(RootErrorHandler);
      } else {
         SetErrorHandler(DefaultErrorHandler);
      }
   }

   void completeRootHandlers()
   {
      // Set ROOT parameters.
      TTree::SetMaxTreeSize(kMaxLong64);
      TH1::AddDirectory(kFALSE);
      G__SetCatchException(0);

      // Initialize tkeyvfs sqlite3 extension for ROOT..
      tkeyvfs_init();

      // Set custom streamers
      setCacheStreamers();
      setProvenanceTransientStreamers();
      detail::setBranchDescriptionStreamer();
      detail::setPtrVectorBaseStreamer();
      configureRefCoreStreamer();
   }

}  // art
