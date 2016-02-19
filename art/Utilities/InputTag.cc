#include "art/Utilities/InputTag.h"

#include "canvas/Utilities/Exception.h"
#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/split.hpp"

namespace art {
  void InputTag::set_from_string_(std::string const& s)
  {
    // string is delimited by colons
    std::vector<std::string> tokens;
    // cet::split(s, ':', std::back_inserter(tokens));
    boost::split(tokens, s, boost::is_any_of(":"), boost::token_compress_off);

    int nwords = tokens.size();
    if(nwords > 3) {
      throw art::Exception(errors::Configuration,"InputTag")
        << "Input tag " << s << " has " << nwords << " tokens";
    }
    if(nwords > 0) label_ = tokens[0];
    if(nwords > 1) instance_ = tokens[1];
    if(nwords > 2) process_=tokens[2];
  }

  std::string InputTag::encode() const {
    //NOTE: since the encoding gets used to form the configuration hash I did not want
    // to change it so that not specifying a process would cause two colons to appear in the
    // encoding and thus not being backwards compatible
    static std::string const separator(":");
    std::string result = label_;
    if(!instance_.empty() || !process_.empty()) {
      result += separator + instance_;
    }
    if(!process_.empty()) {
      result += separator + process_;
    }
    return result;
  }

  std::ostream& operator<<(std::ostream& ost, art::InputTag const& tag) {
    static std::string const process(", process = ");
    ost << "InputTag:  label = " << tag.label()
        << ", instance = " << tag.instance()
        <<(tag.process().empty()?std::string():(process+tag.process()));
    return ost;
  }
}
