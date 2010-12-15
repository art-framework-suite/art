#ifndef FWCore_Services_InitRootHandlers_h
#define FWCore_Services_InitRootHandlers_h

namespace art {

  void unloadRootSigHandler();
  void setRootErrorHandler(bool want_custom);
  void completeRootHandlers(bool want_auto_lib_loader);

}  // namespace art

#endif  // InitRootHandlers_H
