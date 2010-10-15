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
}  // end of namespace art

#endif // InitRootHandlers_H
