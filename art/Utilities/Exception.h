#ifndef art_Utilities_Exception_h
#define art_Utilities_Exception_h

// ======================================================================
//
// Exception: art-specific customizations for cetlib/coded_exception
//
// Note that errors::ErrorCodes is tightly coupled to detail::translate()
// such that any change to one will necessitate a corresponding change to
// the other.  The actions table in Framework/Core/Actions may also need
// adjustment.
//
// ======================================================================

#include "cetlib/coded_exception.h"
#include <string>

// ----------------------------------------------------------------------

namespace art {
  namespace errors {

    enum ErrorCodes {
       OtherArt = 1
    ,  StdException
    ,  Unknown
    ,  BadAlloc
    ,  BadExceptionType

    ,  ProductNotFound
    ,  DictionaryNotFound
    ,  InsertFailure
    ,  Configuration
    ,  LogicError
    ,  UnimplementedFeature
    ,  InvalidReference
    ,  NullPointerError
    ,  NoProductSpecified
    ,  EventTimeout
    ,  DataCorruption

    ,  ScheduleExecutionFailure
    ,  EventProcessorFailure
    ,  EndJobFailure

    ,  FileOpenError
    ,  FileReadError
    ,  FatalRootError
    ,  MismatchedInputFiles

    ,  ProductDoesNotSupportViews
    ,  ProductDoesNotSupportPtr

    ,  InvalidNumber

    ,  NotFound
    };

  }  // errors

  namespace detail {
    std::string
      translate( errors::ErrorCodes );
  }

  typedef  cet::coded_exception<errors::ErrorCodes,detail::translate>
           Exception;
}  // art

// ======================================================================

#endif /* art_Utilities_Exception_h */

// Local Variables:
// mode: c++
// End:
