#ifndef art_Utilities_FirstAbsoluteOrLookupWithDotPolicy_h
#define art_Utilities_FirstAbsoluteOrLookupWithDotPolicy_h

#include "cetlib/filepath_maker.h"

namespace art {
   class FirstAbsoluteOrLookupWithDotPolicy;
}

class art::FirstAbsoluteOrLookupWithDotPolicy :
public cet::filepath_maker {
 public:
   FirstAbsoluteOrLookupWithDotPolicy(std::string const &paths);
   virtual std::string operator() (std::string const &filename);
   void reset();
   virtual ~FirstAbsoluteOrLookupWithDotPolicy() noexcept;

 private:
   bool first;
   cet::search_path first_paths;
   cet::search_path after_paths;

};

#endif /* art_Utilities_FirstAbsoluteOrLookupWithDotPolicy_h */

// Local Variables:
// mode: c++
// End:
