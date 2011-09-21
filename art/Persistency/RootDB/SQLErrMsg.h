#ifndef art_Persistency_RootDB_SQLErrMsg_h
#define art_Persistency_RootDB_SQLErrMsg_h

#include <string>

namespace art {
  class SQLErrMsg;
}

class art::SQLErrMsg {
public:
  SQLErrMsg() : errMsg_(0) { }
  ~SQLErrMsg();

  std::string msg() const { return errMsg_; }

  void
  throwIfError() const;

  void reset();

  operator char ** () { reset(); return &errMsg_; }

private:

  char * errMsg_;
};


#endif /* art_Persistency_RootDB_SQLErrMsg_h */

// Local Variables:
// mode: c++
// End:
