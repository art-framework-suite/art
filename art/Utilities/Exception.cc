
#include "art/Utilities/Exception.h"

using namespace art;

// Map an art::errors::ErrorCodes into the appropriate string.

std::string
detail::translate( errors::ErrorCodes code )
{
  using namespace errors;
  switch( code ) {
  case OtherArt                  : return "OtherArt";
  case StdException              : return "StdException";
  case Unknown                   : return "Unknown";
  case BadAlloc                  : return "BadAlloc";
  case BadExceptionType          : return "BadExceptionType";
  case ProductNotFound           : return "ProductNotFound";
  case DictionaryNotFound        : return "DictionaryNotFound";
  case NoProductSpecified        : return "NoProductSpecified";
  case InsertFailure             : return "InsertFailure";
  case Configuration             : return "Configuration";
  case LogicError                : return "LogicError";
  case UnimplementedFeature      : return "UnimplementedFeature";
  case InvalidReference          : return "InvalidReference";
  case NullPointerError          : return "NullPointerError";
  case EventTimeout              : return "EventTimeout";
  case EventCorruption           : return "EventCorruption";
  case ScheduleExecutionFailure  : return "ScheduleExecutionFailure";
  case EventProcessorFailure     : return "EventProcessorFailure";
  case EndJobFailure             : return "EndJobFailure";
  case FileOpenError             : return "FileOpenError";
  case FileReadError             : return "FileReadError";
  case FatalRootError            : return "FatalRootError";
  case MismatchedInputFiles      : return "MismatchedInputFiles";
  case ProductDoesNotSupportViews: return "ProductDoesNotSupportViews";
  case ProductDoesNotSupportPtr  : return "ProductDoesNotSupportPtr";
  case InvalidNumber             : return "InvalidNumber";
  case NotFound                  : return "NotFound";
  default                        : return "Unknown code";
  }
}
