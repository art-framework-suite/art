#ifndef FWCore_Utilities_RootHandlers_h
#define FWCore_Utilities_RootHandlers_h

namespace art {
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
}  // art

#endif  // FWCore_Utilities_RootHandlers_h
