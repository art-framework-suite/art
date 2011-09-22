#ifndef art_Persistency_RootDB_SQLErrMsg_h
#define art_Persistency_RootDB_SQLErrMsg_h

// Little class to handle the management of ann SQLite3 error message.

// Pass an instance of this class to an SQLite3 function expecting a
// char** into which to place an error message.

#include <string>

namespace art {
  class SQLErrMsg;
}

class art::SQLErrMsg {
public:
  SQLErrMsg() : errMsg_(0) { }
  ~SQLErrMsg();

  // Return the stored message.
  std::string msg() const { return errMsg_; }

  // Throw a suitable message if an error occurred.
  void
  throwIfError() const;

  // Reset and release memory. This will be done automatically in the
  // destructor anyway.
  void reset();

  // Enable an object of this class to be passed to functions expecting
  // char **.
  operator char ** () { reset(); return &errMsg_; }

private:

  char * errMsg_;
};


#endif /* art_Persistency_RootDB_SQLErrMsg_h */

// Local Variables:
// mode: c++
// End:
