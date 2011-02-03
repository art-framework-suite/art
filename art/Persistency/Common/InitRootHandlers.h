#ifndef art_Persistency_Common_InitRootHandlers_h
#define art_Persistency_Common_InitRootHandlers_h

namespace art {

   void unloadRootSigHandler();
   void setRootErrorHandler(bool want_custom);
   void completeRootHandlers();

}  // art

// ======================================================================

#endif /* art_Persistency_Common_InitRootHandlers_h */

// Local Variables:
// mode: c++
// End:
