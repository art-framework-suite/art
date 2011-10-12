#ifndef art_Utilities_InputTag_h
#define art_Utilities_InputTag_h

#include "art/Utilities/fwd.h"

#include <iosfwd>
#include <string>

namespace art {

  class InputTag {
  public:
    InputTag();
    InputTag(std::string const & label, std::string const & instance, std::string const & processName = "");
    InputTag(char const * label, char const * instance, char const * processName = "");
    /// the input string is of the form:
    /// label
    /// label:instance
    /// label:instance:process
    InputTag(std::string const & s);

    // use compiler-generated copy c'tor, copy assignment, and d'tor

    std::string encode() const;

    std::string const & label() const {return label_;}
    std::string const & instance() const {return instance_;}
    ///an empty string means find the most recently produced
    ///product with the label and instance
    std::string const & process() const {return process_;}

    bool operator==(InputTag const & tag) const;

  private:
    std::string label_;
    std::string instance_;
    std::string process_;
  };

  std::ostream & operator<<(std::ostream & ost, InputTag const & tag);

}

#endif /* art_Utilities_InputTag_h */

// Local Variables:
// mode: c++
// End:
