#ifndef art_Utilities_Exception_h
#define art_Utilities_Exception_h

// ======================================================================
//
// exception
//
// ======================================================================

#include "cetlib/coded_exception.h"
#include <string>

// ----------------------------------------------------------------------

namespace art {
  namespace errors {

    // If you add a new entry to the set of values, make sure to
    // update the translation map in Utility/Exception.cc, the actions
    // table in FWCore/Framework/src/Actions.cc, and the configuration
    // fragment FWCore/Framework/test/cmsExceptionsFatalOption.cff.

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
    ,  EventCorruption

    ,  ScheduleExecutionFailure
    ,  EventProcessorFailure
    ,  EndJobFailure

    ,  FileOpenError
    ,  FileReadError
    ,  FatalRootError
    ,  MismatchedInputFiles

    ,  ProductDoesNotSupportViews
    ,  ProductDoesNotSupportPtr

    ,  NotFound
    };

  }  // errors

  namespace detail {
    std::string translate( errors::ErrorCodes );
  }

  typedef  cet::coded_exception<errors::ErrorCodes,detail::translate>
           Exception;
}  // art

// ======================================================================

#endif /* art_Utilities_Exception_h */

// Local Variables:
// mode: c++
// End:
