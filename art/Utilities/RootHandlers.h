#ifndef FWCore_Utilities_RootHandlers_h
#define FWCore_Utilities_RootHandlers_h

namespace edm {
  class RootHandlers {
  public:
    RootHandlers ();
    virtual ~RootHandlers ();
    void disableErrorHandler();
    void enableErrorHandler();
  private:
    virtual void disableErrorHandler_() = 0;
    virtual void enableErrorHandler_() = 0;
  };
}  // end of namespace edm

#endif // InitRootHandlers_H
