#ifndef art_Utilities_FirstAbsoluteOrLookupWithDotPolicy_h
#define art_Utilities_FirstAbsoluteOrLookupWithDotPolicy_h

#include "art/Utilities/fwd.h"

#include "cetlib/filepath_maker.h"
#include "cetlib/search_path.h"

class art::FirstAbsoluteOrLookupWithDotPolicy : public cet::filepath_maker {
public:
  FirstAbsoluteOrLookupWithDotPolicy(std::string const& paths);
  std::string operator()(std::string const& filename) override;
  void reset();

private:
  bool first;
  cet::search_path first_paths;
  cet::search_path after_paths;
};

#endif /* art_Utilities_FirstAbsoluteOrLookupWithDotPolicy_h */

// Local Variables:
// mode: c++
// End:
