#ifndef FWCore_Utilities_ExceptionCollector_h
#define FWCore_Utilities_ExceptionCollector_h

/**
ExceptionCollector is a utility class that can be used to make sure that
each function or functor in a sequence of calls is invoked even if
a previous function throws.  Each function/functor must take no arguments
and return a void.  boost::bind can be used to convert a function taking arguments
into a function taking no arguments.
The exception strings are saved in a cet::exception for optional rethrow.

Here is an example:

ExceptionCollector c;

c.call(boost_bind(&MyClass::myFunction, myClassPtr));
c.call(boost_bind(&MyClass::myOtherFunction, myClassPtr, myArgPtr));
c.call(boost_bind(&myFreeFunction, myArgPtr));
if (c.hasThrown()) c.rethrow();

This insures that all three functions will be called before any exceptionis thrown.
**/

#include "cetlib/exception.h"
#include <exception>
#include "boost/function.hpp"

namespace art {
  class ExceptionCollector {
  public:
    ExceptionCollector() : exception_(std::string()), hasThrown_(false) {}
    ~ExceptionCollector() {}
    bool hasThrown() const {return hasThrown_;}
    void rethrow() const;
    void call(boost::function<void(void)>);

  private:
    cet::exception exception_;
    bool hasThrown_;
  };
}

#endif
