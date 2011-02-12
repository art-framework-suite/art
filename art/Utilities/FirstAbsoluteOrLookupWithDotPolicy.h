#ifndef XX_h
#define XX_h

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
   virtual ~FirstAbsoluteOrLookupWithDotPolicy();

 private:
   bool first;
   cet::search_path first_paths;
   cet::search_path after_paths;

};

#endif
